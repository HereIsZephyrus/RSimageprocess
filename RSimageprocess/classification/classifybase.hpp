//
//  classifybase.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/16/24.
//

#ifndef classifybase_hpp
#define classifybase_hpp
#include <algorithm>
#include <cstring>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include <memory>
#include <Eigen/Dense>
#include <random>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../OpenGL/graphing.hpp"

using std::pair;
using std::unordered_map;
using namespace Eigen;

class ClassMapper{
public:
    static ClassMapper& getClassMap(){
        static ClassMapper instance;
        return instance;
    }
    ClassMapper(const ClassMapper&) = delete;
    void operator = (const ClassMapper) = delete;
    std::string getName(int label) const {return nameMap[label];}
    glm::vec3 getColor(int label) const {return colorMap[label];}
    int getTotalNum() const {return totalNum;}
    static constexpr glm::vec3 blankColor = {0,0,0};
    void generateRandomColorMap(int num);
    void setTotalNum(int num){totalNum = num;}
    std::vector<std::string> nameMap;
    std::vector<glm::vec3> colorMap;
private:
    int totalNum;
    ClassMapper(){}
};
using dataVec = std::vector<float>;
class Sample{
    int label;
    dataVec features;
    bool isTrain;
public:
    Sample(int label,const dataVec& featureData, bool isTrainSample = true)
        :label(label),features(featureData),isTrain(isTrainSample){}
    ~Sample(){}
    int getLabel() const{return label;}
    const dataVec& getFeatures() const{return features;}
    bool isTrainSample() const{return isTrain;}
};
class Classifier;
class Accuracy{
using Dataset = std::vector<Sample>;
friend class Classifier;
protected:
    std::vector<std::vector<int>> confuseMat;
    std::vector<float> f1,precision,recall;
public:
    Accuracy() = default;
    const std::vector<float>& getPrecision() const {return precision;}
    const std::vector<float>& getRecall() const {return recall;}
    const std::vector<float>& getF1() const {return f1;}
    float getComprehensiveAccuracy();
    void PrintPrecision();
};
class Classifier{
protected:
    using Dataset = std::vector<Sample>;
    using ClassMat = std::vector<std::vector<int>>;
    using vClasses = std::vector<int>;
    size_t featureNum;
    std::string classifierName;
    static constexpr int margin = 2;
public:
    Accuracy accuracy;
    Classifier(){classifierName = "classifier";}
    std::string getName() const {return classifierName;}
    void Classify(const std::vector<Band>& bands,unsigned char* classified);
    void Examine(const Dataset& samples);
    virtual void Train(const Dataset &dataset) = 0;
    virtual int Predict(const dataVec& x) = 0;
};
struct ScanLineEdge{
    int y;
    double left,right;
    ScanLineEdge(int y,double left,double right):y(y),left(left),right(right){}
};
void ScanLineEdgeConstruct(std::vector<ScanLineEdge>& edges,std::shared_ptr<ROI> part,OGRCoordinateTransformation *transformation); //only for conv
#endif /* classifybase_hpp */
