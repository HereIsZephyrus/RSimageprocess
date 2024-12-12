//
//  graphing.cpp
//  data_structure
//
//  Created by ChanningTong on 10/22/24.
//

#include <fstream>
#include <sstream>
#include <iostream>
#include <gdal.h>
#include <gdal_priv.h>
#include "graphing.hpp"
#include "window.hpp"
#include "camera.hpp"

std::map<std::string,pShader > ShaderBucket;
GLchar* filePath(const char* fileName){
    //checkSourceRelevantPath();
    const char * shaderSearchPath ="/Users/channingtong/Program/RSimageprocess/RSimageprocess/OpenGL/shaders/";
    GLchar* resource = new char[strlen(shaderSearchPath) + strlen(fileName) + 1];
    strcpy(resource, shaderSearchPath);
    strcat(resource, fileName);
    return resource;
}
std::string Shader::readGLSLfile(const GLchar* filePath){
    std::string fileString;
    std::ifstream fileStream;
    fileStream.exceptions(std::ifstream::badbit);
    try {
        fileStream.open(filePath);
        if (!fileStream.is_open())
            std::cerr << "ERROR::SHADER::Failed_TO_READ_SHADERFILE." << std::endl;
        std::stringstream shaderStream;
        shaderStream << fileStream.rdbuf();
        fileStream.close();
        return shaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cerr<<"ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    return "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ";
}
void Shader::attchShader(std::string shader,GLuint type){
    const GLchar* shaderCode = shader.c_str();
    GLint success;
    GLchar infoLog[512];
    GLuint shaderProgram;
    shaderProgram = glCreateShader(type);
    glShaderSource(shaderProgram, 1, &shaderCode, NULL);
    glCompileShader(shaderProgram);
    // Print compile errors if any
    glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glAttachShader(program, shaderProgram);
    glDeleteShader(shaderProgram);
}
void Shader::attchShader(const GLchar* path,GLuint type){
    std::string shader = readGLSLfile(path);
    attchShader(shader,type);
}
void Shader::linkProgram(){
    GLint success;
    GLchar infoLog[512];
    glLinkProgram(program);
    // Print linking errors if any
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success){
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

Spectum::Spectum(unsigned short* flatd,int w,int h):width(w),height(h){
    rawData = new unsigned short*[height];
    showData = new unsigned char[width * height];
    for (int y = 0; y < height; y++){
        rawData[y] = new unsigned short[width];
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            rawData[y][x] = flatd[loc];
            if (flatd[loc] == 65534)
                showData[loc] = 0; //nodata
            else
                showData[loc] = flatd[loc]/255;
        }
    }
}
Spectum::~Spectum(){
    for (size_t h = 0; h < height; h++)
        delete[] rawData[h];
    delete[] rawData;
    delete[] showData;
}
void Primitive::initResource(GLenum shp,Shader* inputshader){
    transMat = glm::mat4(1.0f);
    shape = shp;
    shader = inputshader;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNum * stride, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
Primitive::Primitive(const std::vector<Vertex>& inputVertex,GLenum shp,Shader* inputshader){
    vertexNum = inputVertex.size();
    vertices = new GLfloat[vertexNum * stride];
    for (size_t i = 0; i < vertexNum; i++){
        vertices[i * stride] = inputVertex[i].position[0];        vertices[i * stride + 1] = inputVertex[i].position[1];        vertices[i * stride + 2] = inputVertex[i].position[2];
        vertices[i * stride + 3] = inputVertex[i].color[0];        vertices[i * stride + 4] = inputVertex[i].color[1];        vertices[i * stride + 5] = inputVertex[i].color[2];
    }
    std::vector<Vertex>::const_iterator vertex = inputVertex.begin();
    extent.left = vertex->position.x;   extent.right = vertex->position.x;
    extent.botton = vertex->position.y;   extent.top = vertex->position.y;
    for (; vertex != inputVertex.end(); vertex++){
        extent.left = std::min(extent.left,vertex->position.x);
        extent.right = std::max(extent.right,vertex->position.x);
        extent.botton = std::min(extent.botton,vertex->position.y);
        extent.top = std::max(extent.top,vertex->position.y);
    }
    initResource(shp,inputshader);
}
Primitive::Primitive(const Vertex& inputVertex,GLenum shp,Shader* inputshader){
    vertexNum = 1;
    vertices = new GLfloat[vertexNum * stride];
    vertices[0] = inputVertex.position[0];        vertices[1] = inputVertex.position[1];        vertices[2] = inputVertex.position[2];
    vertices[3] = inputVertex.color[0];        vertices[4] = inputVertex.color[1];        vertices[5] = inputVertex.color[2];
    extent.left = inputVertex.position.x;   extent.right = inputVertex.position.x + 1;
    extent.botton = inputVertex.position.y;   extent.top = inputVertex.position.y + 1;
    initResource(shp,inputshader);
}
void Primitive::draw() const {
    if (shader == nullptr){
        std::cerr<<"havn't bind shader"<<std::endl;
        return;
    }
    else
        shader ->use();
    GLuint projectionLoc = glGetUniformLocation(shader->program, "projection");
    GLuint viewLoc = glGetUniformLocation(shader->program, "view");
    GLuint modelLoc = glGetUniformLocation(shader->program, "model");
    Camera2D& camera = Camera2D::getView();
    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = transMat;
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    GLuint thicknessLoc = glGetUniformLocation(shader->program, "thickness");
    glUniform1f(thicknessLoc,0.02f);
    glBindVertexArray(VAO);
    glDrawArrays(shape, 0, static_cast<GLsizei>(vertexNum));
    glBindVertexArray(0);
    return;
}
void Primitive::update(){
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNum * stride, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
Texture::Texture(const std::vector<glm::vec3>& position,GLuint textureID):textureID(textureID),shape(GL_TRIANGLE_FAN),shader(ShaderBucket["image"].get()){
    vertexNum = position.size();
    const std::vector<glm::vec2> locatoin = {
        glm::vec2(0.0,1.0), // upleft
        glm::vec2(1.0,1.0), // upright
        glm::vec2(1.0,0.0), // downright
        glm::vec2(0.0,0.0), // downleft
    };
    vertices = new GLfloat[vertexNum * stride];
    for (size_t i = 0; i < vertexNum; i++){
        vertices[i * stride] = position[i].x;        vertices[i * stride + 1] = position[i].y;        vertices[i * stride + 2] = position[i].z;
        vertices[i * stride + 3] = locatoin[i].x;    vertices[i * stride + 4] = locatoin[i].y;
    }
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNum * stride, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void Texture::draw() const {
    glBindTexture(GL_TEXTURE_2D, textureID);
    if (shader == nullptr){
        std::cerr<<"havn't bind shader"<<std::endl;
        return;
    }
    else
        shader ->use();
    GLuint projectionLoc = glGetUniformLocation(shader->program, "projection");
    GLuint viewLoc = glGetUniformLocation(shader->program, "view");
    GLuint modelLoc = glGetUniformLocation(shader->program, "model");
    Camera2D& camera = Camera2D::getView();
    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    GLuint thicknessLoc = glGetUniformLocation(shader->program, "thickness");
    glUniform1f(thicknessLoc,0.02f);
    glBindVertexArray(VAO);
    glDrawArrays(shape, 0, static_cast<GLsizei>(vertexNum));
    glBindVertexArray(0);
    return;
}
Image::Image(std::string resourchPath,const std::vector<Vertex>& faceVertex):Primitive(faceVertex,GL_LINE_LOOP,ShaderBucket["line"].get()){
    
}
void Image::LoadNewBand(std::string searchingPath,std::string spectum){
    GDALDataset *dataset = (GDALDataset *) GDALOpen(searchingPath.c_str(), GA_ReadOnly);
    if (dataset == nullptr) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return;
    }
    GDALRasterBand *band = dataset->GetRasterBand(1);
    int width = band->GetXSize(),height = band->GetYSize();
    unsigned short *data = new unsigned short[width * height];
    band->RasterIO(GF_Read, 0, 0, width, height, data, width, height, GDT_UInt16, 0, 0);
    std::shared_ptr<Spectum> mat = std::make_shared<Spectum>(data,width,height);
    bands.push_back(Band(mat,spectum));
    GDALClose(dataset);
    delete[] data;
}

void Image::draw() const{
    Primitive::draw();
    if (texture != nullptr)
        texture->draw();
}
void Image::generateTexture(int rind, int gind, int bind){
    std::shared_ptr<Spectum> rval = bands[rind].value,gval = bands[gind].value,bval = bands[bind].value;
    const int width = rval->width, height = rval->height;
    std::vector<unsigned char> RGB(width * height * 3);
    std::copy(rval->showData,rval->showData + width * height,RGB.begin());
    std::copy(gval->showData,gval->showData + width * height,RGB.begin() + width * height * 1);
    std::copy(bval->showData,bval->showData + width * height,RGB.begin() + width * height * 2);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, RGB.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    std::vector<glm::vec3> position;
    for (int index = 0; index < vertexNum; index++)
        position.push_back(glm::vec3(vertices[index * stride],vertices[index * stride + 1],vertices[index * stride + 2]));
    texture = std::make_shared<Texture>(position,textureID);
}
void Image::generateTexture(int singleBand){
    std::vector<unsigned char> gray;
    std::shared_ptr<Spectum> value = bands[singleBand].value;
    const int width = value->width, height = value->height;
    gray.assign(value->showData,value->showData + width * height);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, gray.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    std::vector<glm::vec3> position;
    for (int index = 0; index < vertexNum; index++)
        position.push_back(glm::vec3(vertices[index * stride],vertices[index * stride + 1],vertices[index * stride + 2]));
    texture = std::make_shared<Texture>(position,textureID);
}
ROI::ROI(const std::vector<Vertex>& inputVertex):Primitive(inputVertex,GL_LINE_LOOP,ShaderBucket["line"].get()){
    startPosition = inputVertex[0].position;
    
}
ROI::ROI(const Vertex& inputVertex):Primitive(inputVertex,GL_LINE_LOOP,ShaderBucket["line"].get()){
    startPosition = inputVertex.position;
    
}
ROIcollection::ROIcollection(std::string resourchPath){
    
}
void ROIcollection::draw(){
    for (std::vector<ROI>::iterator roi = partition.begin(); roi != partition.end(); roi++)
        roi->draw();
}
Extent ROIcollection::getExtent() const{
    std::vector<ROI>::const_iterator part = partition.begin();
    Extent totalExtent = part->getExtent();
    for (; part != partition.end(); part++){
        Extent thisExtent = part->getExtent();
        totalExtent.left = std::min(totalExtent.left,thisExtent.left);
        totalExtent.right = std::max(totalExtent.right,thisExtent.right);
        totalExtent.botton = std::min(totalExtent.botton,thisExtent.botton);
        totalExtent.top = std::max(totalExtent.top,thisExtent.top);
    }
    return totalExtent;
}
void InitResource(GLFWwindow *window){
    {
        pShader test (new Shader());
        test->attchShader(filePath("test_vertices.vs"),GL_VERTEX_SHADER);
        test->attchShader(filePath("test_line.frag"),GL_FRAGMENT_SHADER);
        test->linkProgram();
        ShaderBucket["test"] = std::move(test);
    }
    {
        pShader line (new Shader());
        line->attchShader(filePath("vertices.vs"),GL_VERTEX_SHADER);
        line->attchShader(filePath("weightingline.gs"), GL_GEOMETRY_SHADER);
        line->attchShader(filePath("line.frag"),GL_FRAGMENT_SHADER);
        line->linkProgram();
        ShaderBucket["line"] = std::move(line);
    }
    {
        pShader image (new Shader());
        image->attchShader(filePath("texture_vertices.vs"),GL_VERTEX_SHADER);
        image->attchShader(filePath("texture.frag"),GL_FRAGMENT_SHADER);
        image->linkProgram();
        ShaderBucket["image"] = std::move(image);
    }
}
