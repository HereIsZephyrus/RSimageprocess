//
//  graphing.hpp
//  data_structure
//
//  Created by ChanningTong on 10/22/24.
//

#ifndef graphing_hpp
#define graphing_hpp

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
#include "camera.hpp"

struct Spectum{
    unsigned short **rawData;
    unsigned char *showData;
    int width,height;
    glm::vec2 validRange[4];
    Spectum(unsigned short* flatd,int w,int h);
    Spectum(const cv::Mat& image);
    ~Spectum();
};
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};
GLchar* filePath(const char* fileName);
class Shader{
public:
    Shader(const Shader&) = delete;
    void operator=(const Shader&) = delete;
    Shader(){
        this->program = glCreateProgram();
    }
    void use(){
        glUseProgram(program);
    }
    void attchShader(std::string shader,GLuint type);
    void attchShader(const GLchar* path,GLuint type);
    void linkProgram();
    GLuint program;
private:
    std::string readGLSLfile(const GLchar* filePath);
};
class Primitive{
public:
    Primitive(const std::vector<Vertex>& inputVertex,GLenum shp,Shader* shader);
    Primitive(const Vertex& inputVertex,GLenum shp,Shader* shader);
    Primitive(const Primitive&) = delete;
    void operator=(const Primitive&) = delete;
    ~Primitive(){
        delete [] vertices;
        glDeleteVertexArrays(1,&VAO);
        glDeleteBuffers(1,&VBO);
    }
    virtual void draw() const;
    void update();
    Extent getExtent() const{return extent;}
protected:
    void initResource(GLenum shp,Shader* inputshader);
    GLuint VAO,VBO;
    Shader* shader;
    GLenum shape;
    static constexpr GLsizei stride = 6;
    size_t vertexNum;
    GLfloat* vertices;
    glm::mat4 transMat;
    Extent extent;
};
class Texture{
public:
    Texture(const std::vector<glm::vec3>& position, const std::vector<glm::vec2>& texturePos, GLuint textureID);
    Texture(const Texture&) = delete;
    void operator=(const Texture&) = delete;
    ~Texture(){
        delete [] vertices;
        glDeleteVertexArrays(1,&VAO);
        glDeleteBuffers(1,&VBO);
        glDeleteTextures(1, &textureID);
    }
    virtual void draw() const;
protected:
    GLuint VAO,VBO,textureID;
    Shader* shader;
    GLenum shape;
    static constexpr GLsizei stride = 5;
    size_t vertexNum;
    GLfloat* vertices;
};
typedef std::unique_ptr<Shader> pShader;
extern std::map<std::string,pShader > ShaderBucket;
void InitResource(GLFWwindow *window);

struct Band{
    std::shared_ptr<Spectum> value;
    std::string  wavelength;
};
class Image : public Primitive{
    std::vector<Band> bands;
    std::shared_ptr<Texture> texture;
public:
    explicit Image(const std::vector<Vertex>& faceVertex):
    Primitive(faceVertex,GL_LINE_LOOP,ShaderBucket["line"].get()),texture(nullptr){}
    void LoadNewBand(std::string searchingPath,std::string wavelength);
    Image(std::string resourchPath,const std::vector<Vertex>& faceVertex);
    void draw() const override;
    const std::vector<Band>& getBands(){return bands;}
    void generateTexture(int rind = 3, int gind = 2, int bind = 1);
    void generateTexture(int singleBand);
};
class ROI : public Primitive{
    glm::vec3 startPosition;
public:
    ROI(const std::vector<Vertex>& inputVertex);
    ROI(const Vertex& inputVertex);
};
class ROIcollection{
    std::vector<ROI> partition;
public:
    ROIcollection(std::string resourchPath);
    void draw();
    Extent getExtent() const;
};
#endif /* graphing_hpp */
