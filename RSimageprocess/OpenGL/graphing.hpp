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
#include <Eigen/Dense>
#include "camera.hpp"

typedef Eigen::MatrixXd Matrix;
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};
GLchar* filePath(const char* fileName);
namespace binarytree{class BallPara;}
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
    friend binarytree::BallPara;
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
typedef std::unique_ptr<Shader> pShader;
extern std::map<std::string,pShader > ShaderBucket;
void InitResource(GLFWwindow *window);
struct Band{
    Matrix value;
    std::string  spectum;
};
class Image : public Primitive{
    std::vector<Band> bands;
    void LoadImage(std::string searchingPath);
public:
    explicit Image(const std::vector<Vertex>& faceVertex):Primitive(faceVertex,GL_LINE_LOOP,ShaderBucket["line"].get()){}
    void LoadNewBand(std::string searchingPath,std::string spectum);
    Image(std::string resourchPath,const std::vector<Vertex>& faceVertex);
    const std::vector<Band>& getBands(){return bands;}
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
