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

void Primitive::initResource(GLenum shp,Shader* inputshader){
    transMat = glm::mat4(1.0f);
    shape = shp;
    shader = inputshader;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNum * 6, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
Primitive::Primitive(const std::vector<Vertex>& inputVertex,GLenum shp,Shader* inputshader){
    using structVert = std::vector<Vertex>;
    vertexNum = inputVertex.size();
    vertices = new GLfloat[vertexNum * stride];
    for (size_t i = 0; i < vertexNum; i++){
        vertices[i * 6] = inputVertex[i].position[0];        vertices[i * 6 + 1] = inputVertex[i].position[1];        vertices[i * 6 + 2] = inputVertex[i].position[2];
        vertices[i * 6 + 3] = inputVertex[i].color[0];        vertices[i * 6 + 4] = inputVertex[i].color[1];        vertices[i * 6 + 5] = inputVertex[i].color[2];
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexNum * 6, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*stride, (GLvoid*)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
Image::Image(std::string resourchPath,const std::vector<Vertex>& faceVertex):Primitive(faceVertex,GL_LINE_LOOP,ShaderBucket["line"].get()){
    
}
void Image::LoadImage(std::string searchingPaht){
    
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
}
