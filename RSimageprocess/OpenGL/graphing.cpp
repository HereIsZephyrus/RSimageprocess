//
//  graphing.cpp
//  data_structure
//
//  Created by ChanningTong on 10/22/24.
//

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <gdal.h>
#include <gdal_priv.h>
#include "graphing.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "commander.hpp"

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

Spectum::Spectum(unsigned short* flatd,int w,int h):width(w),height(h){\
    if (width % 2) width--;
    if (height%2)height--;
    rawData = new unsigned short*[height];
    std::array<int, SPECT_VALUE_RANGE> counting;
    counting = {0};
    CDF = {0.0f};
    minVal = SPECT_VALUE_RANGE - 1;
    maxVal = 0;
    double sum = 0;
    for (int y = 0; y < height; y++){
        rawData[y] = new unsigned short[width];
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            rawData[y][x] = flatd[loc];
            ++counting[rawData[y][x]];
            if (rawData[y][x] == 0)
                continue;
            if (minVal > rawData[y][x]) minVal = rawData[y][x];
            if (minVal < rawData[y][x]) maxVal = rawData[y][x];
        }
    }
    totalPixel = width * height - counting[0];
    mean = sum / totalPixel;
    CDF[0] = counting[0];
    HistHeight = 0;
    for (int val = 1; val < SPECT_VALUE_RANGE; val++){
        hist[val] = static_cast<float>(counting[val]) / totalPixel;
        if (hist[val] > HistHeight) HistHeight = hist[val];
        counting[val] = counting[val-1] + counting[val];
        CDF[val] = static_cast<double>(counting[val]) / totalPixel;
    }
    strechRange.first = 0; strechRange.second = SPECT_VALUE_RANGE - 1;\
}
Spectum::Spectum(const cv::Mat& image){
    width = image.cols; height = image.rows;\
    if (width % 2) width--;
    if (height%2)height--;
    rawData = new unsigned short*[height];
    std::array<int, SPECT_VALUE_RANGE> counting;
    counting = {0};
    CDF = {0.0f};
    minVal = SPECT_VALUE_RANGE - 1;
    maxVal = 0;
    double sum = 0;
    for (int y = 0; y < height; y++){
        rawData[y] = new unsigned short[width];
        for (int x = 0; x < width; x++){
            rawData[y][x] = image.at<ushort>(y,x);
            sum += rawData[y][x];
            ++counting[rawData[y][x]];
            if (rawData[y][x] == 0)
                continue;
            if (minVal > rawData[y][x]) minVal = rawData[y][x];
            if (minVal < rawData[y][x]) maxVal = rawData[y][x];
        }
    }
    totalPixel = width * height - counting[0];
    mean = sum / totalPixel;
    CDF[0] = counting[0] = 0;
    for (int val = 1; val < SPECT_VALUE_RANGE; val++){
        hist[val] = static_cast<float>(counting[val]) / totalPixel;
        if (hist[val] > HistHeight) HistHeight = hist[val];
        counting[val] = counting[val-1] + counting[val];
        CDF[val] = static_cast<double>(counting[val]) / totalPixel;
    }
    strechRange.first = 0; strechRange.second = SPECT_VALUE_RANGE - 1;
}
unsigned short Spectum::average(int y,int x){
    return static_cast<unsigned short>(CDF[rawData[y][x]] * (SPECT_VALUE_RANGE - 1));
}
unsigned short Spectum::strech(int y,int x){
    double streched = static_cast<double>(rawData[y][x] - strechRange.first) / (strechRange.second - strechRange.first) * (SPECT_VALUE_RANGE - 1);
    if (streched < 0)                       return 0;
    if (streched > SPECT_VALUE_RANGE - 1)   return SPECT_VALUE_RANGE - 1;
    return static_cast<unsigned short>(streched);
}
SpectumRange Spectum::setStrech(StrechLevel level){
    switch (level) {
        case StrechLevel::noStrech:{
            strechRange.first = 0;
            strechRange.second = SPECT_VALUE_RANGE - 1;
            break;
        }
        case StrechLevel::minmaxStrech:{
            for (int i = 0; i < SPECT_VALUE_RANGE; i++)
                if (CDF[i] > 0){
                    if (i == 0)
                        strechRange.first = i;
                    else
                        strechRange.first = i - 1;
                    break;
                }
            for (int i = SPECT_VALUE_RANGE - 1; i >=0; i--)
                if (CDF[i] < 1){
                    if (i == SPECT_VALUE_RANGE - 1)
                        strechRange.second = i;
                    else
                        strechRange.second = i + 1;
                    break;
                }
            break;
        }
        case StrechLevel::percent1Strech:{
            for (int i = 0; i < SPECT_VALUE_RANGE; i++)
                if (CDF[i] > 0.01){
                    strechRange.first = i;
                    break;
                }
            for (int i = SPECT_VALUE_RANGE - 1; i >=0; i--)
                if (CDF[i] < 0.99){
                    strechRange.second = i;
                    break;
                }
            break;
        }
        case StrechLevel::percent2Strech:{
            for (int i = 0; i < SPECT_VALUE_RANGE; i++)
                if (CDF[i] > 0.02){
                    strechRange.first = i;
                    break;
                }
            for (int i = SPECT_VALUE_RANGE - 1; i >=0; i--)
                if (CDF[i] < 0.98){
                    strechRange.second = i;
                    break;
                }
            break;
        }
    }
    return strechRange;
}
Spectum::~Spectum(){
    for (size_t h = 0; h < height; h++)
        delete[] rawData[h];
    delete[] rawData;
    //delete[] showData;
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
Texture::Texture(const std::vector<glm::vec3>& position, GLuint textureID,bool useRGB):
textureID(textureID),shape(GL_TRIANGLE_FAN){
    std::array<glm::vec2, 4> location{
        glm::vec2(0.0,1.0),
        glm::vec2(1.0,1.0),
        glm::vec2(1.0,0.0),
        glm::vec2(0.0,0.0),
    };
    if (useRGB)
        shader = ShaderBucket["RGB"].get();
    else
        shader = ShaderBucket["Gray"].get();
    vertexNum = position.size();
    vertices = new GLfloat[vertexNum * stride];
    for (size_t i = 0; i < vertexNum; i++){
        vertices[i * stride] = position[i].x;        vertices[i * stride + 1] = position[i].y;        vertices[i * stride + 2] = position[i].z;
        vertices[i * stride + 3] = location[i].x;    vertices[i * stride + 4] = location[i].y;
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
    glBindTexture(GL_TEXTURE_2D, textureID);
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
    glActiveTexture(GL_TEXTURE0);
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
    glUniform1i(glGetUniformLocation(shader->program, "textureSampler"), 0);
    GLuint thicknessLoc = glGetUniformLocation(shader->program, "thickness");
    glUniform1f(thicknessLoc,0.02f);
    glBindVertexArray(VAO);
    glDrawArrays(shape, 0, static_cast<GLsizei>(vertexNum));
    glBindVertexArray(0);
    return;
}
std::string BandProcess::printParas(){
    std::string paraStr = "bandwidth = " + std::to_string(static_cast<int>(paras.at("bandwidth")));
    return paraStr;
}
void BandProcess::execute(Matrix& input,Matrix& output) const{
    switch (type) {
        case BandProcessType::meanBlur:
            executeMeanBlur(input, output);
            break;
        case BandProcessType::gaussianBlur:
            executeGaussianBlur(input, output);
            break;
        case BandProcessType::laplacian:
            executeLaplacianBlur(input, output);
            break;
        case BandProcessType::sobel:
            executeSobelBlur(input, output);
            break;
    }
}
void BandProcess::executeMeanBlur(Matrix& input,Matrix& output) const{
    const int bandwidth = static_cast<int>(paras.at("bandwidth")), margin = bandwidth / 2;
    const size_t width = input[0].size(),height = input.size();
    for (size_t y = 0; y < height; y++) {
        size_t ystart = 0, yterm = height - 1;
        if (y >= margin) ystart = y - margin;
        if (y < height - margin)   yterm = y + margin;
        for (size_t x = 0; x < width; x++){
            if (input[y][x] == 0)
                continue;
            size_t xstart = 0, xterm = width - 1;
            if (x >= margin) xstart = x - margin;
            if (x < height - margin)   xterm = x + margin;
            double sum = 0;
            int count = 0;
            for (size_t i = ystart; i <= yterm; i++)
                for (size_t j = xstart; j <= xterm; j++){
                    sum += input[i][j];
                    ++count;
                }
            output[y][x] = static_cast<double>(sum / count);
        }
    }
}
void BandProcess::executeGaussianBlur(Matrix& input,Matrix& output) const{
    const int bandwidth = static_cast<int>(paras.at("bandwidth")), margin = bandwidth / 2;
    const size_t width = input[0].size(),height = input.size();
    std::vector<std::vector<double>> kernel(bandwidth,std::vector<double>(bandwidth,0));
    double sum = 0;
    for (int y = -margin; y <= margin; y++)
        for (int x = -margin; x <= margin; x++){
            double sqrSigma = static_cast<double>(bandwidth * bandwidth / 2);
            double exponent = static_cast<double>(-(x * x + y * y)) / sqrSigma;
            kernel[y + margin][x + margin] = exp(exponent) / (M_PI * sqrSigma);
            sum += kernel[y + margin][x + margin];
        }
    for (int y = -margin; y <= margin; y++)
        for (int x = -margin; x <= margin; x++)
            kernel[y + margin][x + margin] /= sum;
    for (size_t y = 0; y < height; y++) {
        size_t ystart = 0, yterm = height - 1;
        if (y >= margin) ystart = y - margin;
        if (y < height - margin)   yterm = y + margin;
        for (size_t x = 0; x < width; x++){
            if (input[y][x] == 0)
                continue;
            size_t xstart = 0, xterm = width - 1;
            if (x >= margin) xstart = x - margin;
            if (x < height - margin)   xterm = x + margin;
            double sum = 0.0;
            for (size_t i = ystart; i <= yterm; i++)
                for (size_t j = xstart; j <= xterm; j++)
                    sum += input[i][j] * kernel[i - y + margin][j -x + margin];
            output[y][x] = static_cast<unsigned short>(sum);
        }
    }
}
void BandProcess::executeLaplacianBlur(Matrix& input,Matrix& output) const{
    const int bandwidth = static_cast<int>(paras.at("bandwidth")), margin = bandwidth / 2;
    const size_t width = input[0].size(),height = input.size();
    std::vector<std::vector<double>> kernel(bandwidth,std::vector<double>(bandwidth,0));
    for (size_t y = 0; y < bandwidth; y++)
        kernel[y][margin] = -1;
    for (size_t x = 0; x < bandwidth; x++)
        kernel[margin][x] = -1;
    kernel[margin][margin] = 4 * margin;
    for (size_t y = 0; y < height; y++) {
        size_t ystart = 0, yterm = height - 1;
        if (y >= margin) ystart = y - margin;
        if (y < height - margin)   yterm = y + margin;
        for (size_t x = 0; x < width; x++){
            if (input[y][x] == 0)
                continue;
            size_t xstart = 0, xterm = width - 1;
            if (x >= margin) xstart = x - margin;
            if (x < height - margin)   xterm = x + margin;
            double sum = 0.0;
            for (size_t i = ystart; i <= yterm; i++){
                for (size_t j = xstart; j <= xterm; j++){
                    sum += input[i][j] * kernel[i - y + margin][j -x + margin];
                    //std::cout<< input[i][j]<<' ';
                }
                //std::cout<<std::endl;
            }
            if (abs(sum) > SPECT_VALUE_RANGE - 1)
                output[y][x] = SPECT_VALUE_RANGE - 1;
            else
                output[y][x] = static_cast<unsigned short>(abs(sum));
        }
    }
}
void BandProcess::executeSobelBlur(Matrix& input,Matrix& output) const{
    const int bandwidth = static_cast<int>(paras.at("bandwidth")), margin = bandwidth / 2;
    const size_t width = input[0].size(),height = input.size();
    std::vector<std::vector<double>> kernelx(bandwidth,std::vector<double>(bandwidth,0)),kernely(bandwidth,std::vector<double>(bandwidth,0));
    for (int i  = 0; i <= margin; i++){
        for (int j = 0; j <= margin; j++){
            kernelx[margin - i][margin - j] = (margin - i + 1) *(-j);
            kernelx[margin - i][margin + j] = (margin - i + 1) *(j);
            kernelx[margin + i][margin - j] = (margin - i + 1) *(-j);
            kernelx[margin + i][margin + j] = (margin - i + 1) *(j);
            
            kernely[margin - i][margin - j] = (margin - j + 1) *(-i);
            kernely[margin - i][margin + j] = (margin - j + 1) *(-i);
            kernely[margin + i][margin - j] = (margin - j + 1) *(i);
            kernely[margin + i][margin + j] = (margin - j + 1) *(i);
        }
    }
    for (size_t y = 0; y < height; y++) {
        size_t ystart = 0, yterm = height - 1;
        if (y >= margin) ystart = y - margin;
        if (y < height - margin)   yterm = y + margin;
        for (size_t x = 0; x < width; x++){
            if (input[y][x] == 0)
                continue;
            size_t xstart = 0, xterm = width - 1;
            if (x >= margin) xstart = x - margin;
            if (x < height - margin)   xterm = x + margin;
            double sumX = 0.0, sumY = 0.0;
            for (size_t i = ystart; i <= yterm; i++)
                for (size_t j = xstart; j <= xterm; j++){
                    sumX += input[i][j] * kernelx[i - y + margin][j -x + margin];
                    sumY += input[i][j] * kernely[i - y + margin][j -x + margin];
                }
            double gredient = std::sqrt(sumX * sumX + sumY * sumY);
            if (gredient > SPECT_VALUE_RANGE - 1)
                output[y][x] = SPECT_VALUE_RANGE - 1;
            else
                output[y][x] = gredient;
        }
    }
}
void TextureManager::processBand(unsigned short* RGB,std::shared_ptr<Spectum> band, int bias, const std::vector<BandProcess>& processes){
    const int width = band->width,height = band->height;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            if (toAverage)
                RGB[loc * 3 + bias] = band->average(y,x);
            else
                RGB[loc * 3 + bias] = band->strech(y,x);
        }
    if (processes.empty())
        return;
    std::vector<std::vector<unsigned short>> buffers[2];
    for (int y = 0; y < height; y++){
        std::vector<unsigned short> row1(width),row2(width);
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            row1[x] = RGB[loc * 3 + bias];
            row2[x] = 0;
        }
        buffers[0].push_back(row1);
        buffers[1].push_back(row2);
    }
    bool swapbuffer = false;
    for (std::vector<BandProcess>::const_iterator process = processes.begin(); process != processes.end(); process++){
        if (!swapbuffer)
            process->execute(buffers[0],buffers[1]);
        else
            process->execute(buffers[1],buffers[0]);
        swapbuffer = !swapbuffer;
    }
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            if (swapbuffer)
                RGB[loc * 3 + bias] = buffers[1][y][x];
            else
                RGB[loc * 3 + bias] = buffers[0][y][x];
        }
}
void TextureManager::processBand(unsigned short* Gray,std::shared_ptr<Spectum> band, const std::vector<BandProcess>& processes){
    const int width = band->width,height = band->height;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            if (toAverage)
                Gray[loc] = band->average(y,x);
            else
                Gray[loc] = band->strech(y,x);
        }
    if (processes.empty())
        return;
    std::vector<std::vector<unsigned short>> buffers[2];
    for (int y = 0; y < height; y++){
        std::vector<unsigned short> row1(width),row2(width);
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            row1[x] = Gray[loc];
            row2[x] = 0;
        }
        buffers[0].push_back(row1);
        buffers[1].push_back(row2);
    }
    bool swapbuffer = false;
    for (std::vector<BandProcess>::const_iterator process = processes.begin(); process != processes.end(); process++){
        if (!swapbuffer)
            process->execute(buffers[0],buffers[1]);
        else
            process->execute(buffers[1],buffers[0]);
        swapbuffer = !swapbuffer;
    }
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            if (swapbuffer)
                Gray[loc] = buffers[1][y][x];
            else
                Gray[loc] = buffers[0][y][x];
        }
}
std::string TextureManager::getStatus(){
    std::string status = "_" + std::to_string(RGBindex[0]) + std::to_string(RGBindex[1]) + std::to_string(RGBindex[2]);
    if (toAverage)
        status += "_aver";
    static const std::unordered_map<BandProcessType,std::string> methodList{
        {BandProcessType::meanBlur,"_meanBlur"},
        {BandProcessType::gaussianBlur,"_gaussianBlur"},
        {BandProcessType::laplacian,"_Laplacian"},
        {BandProcessType::sobel,"_Sobel"},
    };
    const std::vector<BandProcess>& processes = BufferRecorder::getBuffer().processes;
    for (std::vector<BandProcess>::const_iterator process = processes.begin(); process != processes.end(); process++){
        status += methodList.at(process->getType());
    }
    return status;
}
double Image::calcCoefficent(size_t bandind1,size_t bandind2){
    unsigned short **banddata1 = bands[bandind1].value->rawData;
    unsigned short **banddata2 = bands[bandind2].value->rawData;
    int width = bands[bandind1].value->width, height = bands[bandind1].value->height;
    int totalPixel = bands[bandind1].value->totalPixel;
    double mean1 = bands[bandind1].value->mean, mean2 = bands[bandind2].value->mean;
    double covariance = 0.0;
    double variance1 = 0.0, variance2 = 0.0;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            if (banddata1[y][x] == 0 || banddata2[y][x] == 0)
                continue;;
            covariance += (banddata1[y][x] - mean1) * (banddata2[y][x] - mean2);
            variance1 += (banddata1[y][x] - mean1) * (banddata1[y][x] - mean1);
            variance2 += (banddata2[y][x] - mean2) * (banddata2[y][x] - mean2);
        }
    covariance /= totalPixel;
    variance1 /= totalPixel;
    variance2 /= totalPixel;
    double stddev1 = std::sqrt(variance1);
    double stddev2 = std::sqrt(variance2);
    double coefficent = covariance / (stddev1 * stddev2);
    return coefficent;
}
void Image::calcBandCoefficent(){
    const size_t size = bands.size();
    correlation = new double*[size];
    for (size_t i = 0; i < size; i++)
        correlation[i] = new double[size];
    for (size_t i = 0; i < size; i++){
        correlation[i][i] = 1;
        for (size_t j = i + 1; j < size; j++){
            double coefficent = calcCoefficent(i, j);
            correlation[i][j] = coefficent;
            correlation[j][i] = coefficent;
        }
    }
}
void Image::showBandCoefficient(){
    const int size = static_cast<int>(bands.size());
    if (correlation == nullptr)
        calcBandCoefficent();
    if (ImGui::BeginTable("##coefficent", size + 1, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("波段数");
        for (int i = 0; i < size; i++)
            ImGui::TableSetupColumn(std::string("波段" + std::to_string(i + 1)).c_str());
        ImGui::TableHeadersRow();
        for (int i = 0; i < size; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", std::string("波段" + std::to_string(i + 1)).c_str());
            for (int j = 0; j < size; j++) {
                ImGui::TableNextColumn();
                std::stringstream oss;
                oss<<std::setprecision(2)<<correlation[i][j];
                ImGui::Text("%s", oss.str().c_str());
            }
        }
    }
    ImGui::EndTable();
}
void Image::showBandInfo(int bandindex){
    ImGui::Text("%s",std::string("最大值" + std::to_string(bands[bandindex].value->maxVal)).c_str());
    ImGui::SameLine();
    ImGui::Text("%s",std::string("最小值" + std::to_string(bands[bandindex].value->minVal)).c_str());
    ImGui::SameLine();
    ImGui::Text("%s",std::string("平均值" + std::to_string(bands[bandindex].value->mean)).c_str());
    ImGui::PlotHistogram("##直方图数据", bands[bandindex].value->hist.data(), static_cast<int>(bands[bandindex].value->hist.size()), 0, nullptr, 0.0f, bands[bandindex].value->HistHeight, ImVec2(0, 150));
}
void Image::manageBands() {
    static constexpr std::array<std::string,3> colormap = {"red","green","blue"};
    ImGui::OpenPopup("Manage Bands");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(pos);
    if (ImGui::BeginPopup("Manage Bands")) {
        if (textureManager.pointIndex < textureManager.bandNum[textureManager.useRGB]){
            std::string selectInfo = "Select the " + colormap[textureManager.pointIndex] + " band:";
            if (textureManager.useRGB == false)
                selectInfo = "Select view band:";
            ImGui::Text("%s", selectInfo.c_str());
        }
        int counter = 0;
        std::vector<std::string> bandStrVec;
        for (std::vector<Band>::const_reverse_iterator band = bands.rbegin(); band != bands.rend(); band++){
            std::ostringstream nameOS;
            nameOS<<"band"<<++counter<<std::setprecision(1)<<":"<<band->wavelength<<"mm";
            bandStrVec.push_back(nameOS.str());
        }
        std::vector<const char*> bandCharVec;
        for (std::vector<std::string>::const_iterator bandStr = bandStrVec.begin(); bandStr != bandStrVec.end(); bandStr++)
            bandCharVec.push_back(bandStr->c_str());
        int selectedItem = -1;
        if (ImGui::ListBox("##loaded bands", &selectedItem, bandCharVec.data(), static_cast<int>(bandCharVec.size()), 7) && textureManager.pointIndex < textureManager.bandNum[textureManager.useRGB]){
            if (textureManager.useRGB)
                textureManager.RGBindex[textureManager.pointIndex] = selectedItem;
            else
                textureManager.grayIndex = selectedItem;
            ++textureManager.pointIndex;
        }
        ImGui::PushFont(gui::chineseFont);
        static constexpr std::array<std::string, 2> imgeTypeStr{"灰度图像","RGB合成"};
        if (ImGui::BeginCombo("加载类型", imgeTypeStr[textureManager.useRGB].c_str())) {
            for (int i = 0; i < imgeTypeStr.size(); i++) {
                bool isSelected = (selectedItem == i);
                if (ImGui::Selectable(imgeTypeStr[i].c_str(), isSelected)){
                    if (textureManager.useRGB != i){
                        textureManager.pointIndex = 0;
                        textureManager.useRGB = i;
                    }
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (textureManager.pointIndex >= textureManager.bandNum[textureManager.useRGB]){
            if (ImGui::Button("确认")) {
                deleteTexture();
                generateTexture({});
                gui::toShowManageBand = false;
            }
        }
        ImGui::PopFont();
        ImGui::EndPopup();
    }
}
void Image::averageBands(){
    textureManager.setToAverage(true);
    deleteTexture();
    generateTexture({});
}
void Image::strechBands(StrechLevel level,bool useGlobalRange) {
    SpectumRange globalRange{65535,0};
    for (std::vector<Band>::iterator band = bands.begin(); band != bands.end(); band++){
        SpectumRange bandRange = band->value->setStrech(level);
        globalRange.first = std::min(globalRange.first,bandRange.first);
        globalRange.second = std::max(globalRange.second,bandRange.second);
    }
    if (useGlobalRange)
        for (std::vector<Band>::iterator band = bands.begin(); band != bands.end(); band++)
            band->value->strechRange = globalRange;
    textureManager.setToAverage(false);
    deleteTexture();
    generateTexture({});
}
void Image::exportImage(std::string filePath){
    if (textureManager.useRGB)
        exportRGBImage(filePath);
    else
        exportGrayImage(filePath);
}
void Image::exportRGBImage(std::string filePath){
    const int width = bands[0].value->width, height = bands[0].value->height;
    cv::Mat rChannel = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat gChannel = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat bChannel = cv::Mat::zeros(height, width, CV_8UC1);
    std::shared_ptr<Spectum> rval = bands[textureManager.RGBindex[0]].value;
    std::shared_ptr<Spectum> gval = bands[textureManager.RGBindex[1]].value;
    std::shared_ptr<Spectum> bval = bands[textureManager.RGBindex[2]].value;
    unsigned short *RGB = new unsigned short[width * height * 3];
    const std::vector<BandProcess>& processes = BufferRecorder::getBuffer().processes;
    textureManager.processBand(RGB,bval,0,processes);
    textureManager.processBand(RGB,gval,1,processes);
    textureManager.processBand(RGB,rval,2,processes);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            rChannel.at<uchar>(y,x) = static_cast<uchar>(static_cast<float>(RGB[loc * 3 + 0]) * 255 / 65535);
            gChannel.at<uchar>(y,x) = static_cast<uchar>(static_cast<float>(RGB[loc * 3 + 1]) * 255 / 65535);
            bChannel.at<uchar>(y,x) = static_cast<uchar>(static_cast<float>(RGB[loc * 3 + 2]) * 255 / 65535);
        }
    delete[] RGB;
    cv::Mat rgbImage;
    std::vector<cv::Mat> channels = {rChannel, gChannel, bChannel};
    cv::merge(channels, rgbImage);
    cv::imwrite(filePath.c_str(), rgbImage);
}
void Image::exportGrayImage(std::string filePath){
    const int width = bands[0].value->width, height = bands[0].value->height;
    cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1);
    std::shared_ptr<Spectum> val = bands[textureManager.grayIndex].value;
    unsigned short *Gray = new unsigned short[width * height];
    const std::vector<BandProcess>& processes = BufferRecorder::getBuffer().processes;
    textureManager.processBand(Gray,val,processes);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            if (textureManager.getToAverage())
                image.at<uchar>(y,x) = static_cast<uchar>(static_cast<float>(Gray[y * width + x]) * 255 / 65535);
            else
                image.at<uchar>(y,x) = static_cast<uchar>(static_cast<float>(Gray[y * width + x]) * 255 / 65535);
        }
    cv::imwrite(filePath.c_str(), image);
}
std::string TextureManager::getIndicator(int index){
    std::string indicator = "";
    if (pointIndex < bandNum[useRGB])
        return indicator;
    if (useRGB){
        if (RGBindex[0] == index)    return "(R)";
        if (RGBindex[1] == index)    return "(G)";
        if (RGBindex[2] == index)    return "(B)";
    }else{
        if (grayIndex == index) return"(S)";
    }
    return indicator;
}
void Image::LoadNewBand(std::string searchingPath,std::string spectum){
    cv::Mat image = cv::imread(searchingPath.c_str(),cv::IMREAD_UNCHANGED);
    std::shared_ptr<Spectum> mat = std::make_shared<Spectum>(image);
    bands.push_back(Band(mat,spectum));
    /*
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
     */
}

void Image::draw() const{
    Primitive::draw();
    textureManager.draw();
}
void Image::generateTexture(const std::vector<BandProcess>& processes){
    const int width = bands[0].value->width, height = bands[0].value->height;
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (textureManager.useRGB){
        std::shared_ptr<Spectum> rval = bands[textureManager.RGBindex[0]].value;
        std::shared_ptr<Spectum> gval = bands[textureManager.RGBindex[1]].value;
        std::shared_ptr<Spectum> bval = bands[textureManager.RGBindex[2]].value;
        unsigned short *RGB = new unsigned short[width * height * 3];
        textureManager.processBand(RGB,bval,0,processes);
        textureManager.processBand(RGB,gval,1,processes);
        textureManager.processBand(RGB,rval,2,processes);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT, RGB);
        delete[] RGB;
    }else{
        std::shared_ptr<Spectum> val = bands[textureManager.grayIndex].value;
        unsigned short *Gray = new unsigned short[width * height];
        textureManager.processBand(Gray,val,processes);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, Gray);
        delete[] Gray;
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    std::vector<glm::vec3> position;
    for (int index = 0; index < vertexNum ; index++)
            position.push_back(glm::vec3(vertices[index * stride],vertices[index * stride + 1],vertices[index * stride + 2]));
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(position,textureID,textureManager.useRGB);
    textureManager.createtexture(texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Image::generateClassifiedTexture(unsigned char *classified){
    const int width = bands[0].value->width, height = bands[0].value->height;
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT, classified);
    delete[] classified;
    glGenerateMipmap(GL_TEXTURE_2D);
    std::vector<glm::vec3> position;
    for (int index = 0; index < vertexNum ; index++)
            position.push_back(glm::vec3(vertices[index * stride],vertices[index * stride + 1],vertices[index * stride + 2]));
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(position,textureID,textureManager.useRGB);
    textureManager.createtexture(texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}
static bool sortVertexCmp(const glm::vec2& a, const glm::vec2& b){
    if (a.y != b.y) return a.y < b.y;
    return a.x < b.x;
}
void ROI::getSortedVertex(std::vector<glm::vec2>& sorted,OGRCoordinateTransformation *transformation){
    for (int i = 0; i < vertexNum; i++){
        double lat = vertices[i * stride + 1], lon = vertices[i * stride];
        if (transformation->Transform(1, &lat, &lon))
            sorted.push_back(glm::vec2(lat,lon));
    }
    std::sort(sorted.begin(),sorted.end(),sortVertexCmp);
}
void ROIcollection::draw(){
    for (std::vector<ROIobject>::iterator roi = roiCollection.begin(); roi != roiCollection.end(); roi++)
        for (std::vector<std::shared_ptr<ROI>>::iterator part = roi->partition.begin(); part != roi->partition.end(); part++){
            (*part)->draw();
        }
}
/*
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
 */
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
        image->attchShader(filePath("texture_RGB.frag"),GL_FRAGMENT_SHADER);
        image->linkProgram();
        ShaderBucket["RGB"] = std::move(image);
    }
    {
        pShader image (new Shader());
        image->attchShader(filePath("texture_vertices.vs"),GL_VERTEX_SHADER);
        image->attchShader(filePath("texture_Gray.frag"),GL_FRAGMENT_SHADER);
        image->linkProgram();
        ShaderBucket["Gray"] = std::move(image);
    }
}
