//
//  graphing.hpp
//  data_structure
//
//  Created by ChanningTong on 10/22/24.
//

#ifndef graphing_hpp
#define graphing_hpp

#define GLEW_STATIC
#define SPECT_VALUE_RANGE 65536
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <string>
#include <map>
#include <array>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
#include "camera.hpp"

enum class StrechLevel{
    noStrech,
    minmaxStrech,
    percent1Strech,
    percent2Strech,
};
typedef std::pair<unsigned short,unsigned short> SpectumRange;
struct Spectum{
    unsigned short **rawData;
    unsigned short maxVal,minVal;
    int width,height,totalPixel;
    double mean;
    glm::vec2 validRange[4];
    Spectum(unsigned short* flatd,int w,int h); //deprecated at this time
    Spectum(const cv::Mat& image);
    SpectumRange strechRange;
    std::array<float,SPECT_VALUE_RANGE> CDF,hist;
    float HistHeight;
    unsigned short average(int y,int x);
    unsigned short strech(int y,int x);
    SpectumRange setStrech(StrechLevel level);
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

enum class BandProcessType{
    meanBlur,
    gaussianBlur,
    laplacian,
    sobel
};
class BandProcess{
    using Matrix = std::vector<std::vector<unsigned short>>;
    BandProcessType type;
    std::map<std::string,int> paras;
    void executeMeanBlur(Matrix& input,Matrix& output) const;
    void executeGaussianBlur(Matrix& input,Matrix& output) const;
    void executeLaplacianBlur(Matrix& input,Matrix& output) const;
    void executeSobelBlur(Matrix& input,Matrix& output) const;
public:
    void execute(Matrix& input,Matrix& output) const;
};
class TextureManager{
using pTexture = std::shared_ptr<Texture>;
    pTexture texture;
    bool toAverage;
public:
    int RGBindex[3],pointIndex;
    TextureManager(pTexture texturePtr) : texture(texturePtr),pointIndex(3),toAverage(false){
        RGBindex[0] = 3; //red
        RGBindex[1] = 2; //green
        RGBindex[2] = 1; //blue
    }
    void draw() const{
        if (texture != nullptr)
            texture->draw();
    }
    void deleteTexture() {texture = nullptr;}
    void createtexture(pTexture texturePtr) {texture = texturePtr;}
    void processBand(unsigned short* RGB,std::shared_ptr<Spectum> band, int bias, const std::vector<BandProcess>& processes);
    void setToAverage(bool status) {toAverage = status;}
    bool getToAverage() const{return toAverage;}
    std::string getStatus();
};
struct Band{
    std::shared_ptr<Spectum> value;
    std::string  wavelength;
};
class Image : public Primitive{
    std::vector<Band> bands;
    TextureManager textureManager;
    double **correlation;
    void calcBandCoefficent();
    double calcCoefficent(size_t bandind1,size_t bandind2);
public:
    explicit Image(const std::vector<Vertex>& faceVertex):
    Primitive(faceVertex,GL_LINE_LOOP,ShaderBucket["line"].get()),textureManager(nullptr),correlation(nullptr){}
    ~Image(){
        if (correlation != nullptr){
            for (size_t i = 0; i < bands.size(); i++)
                delete[] correlation[i];
            delete[] correlation;
        }
    }
    void LoadNewBand(std::string searchingPath,std::string wavelength);
    void draw() const override;
    const std::vector<Band>& getBands(){return bands;}
    void generateTexture(const std::vector<BandProcess>& processes);
    void deleteTexture() {textureManager.deleteTexture();}
    void exportImage(std::string filePath);
    void manageBands();
    void averageBands();
    void strechBands(StrechLevel level,bool useGlobalRange);
    void resetIndex() {textureManager.pointIndex = 0;}
    void showBandInfo(int bandIndex);
    void showBandCoefficient();
    std::string getTextureStatus(){return textureManager.getStatus();}
    std::string getIndicator(int bandindex);
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
