#ifndef TCLASSIFIER_HPP
#define TCLASSIFIER_HPP
#include "classifybase.hpp"

struct BasicBayesParaList{
    float w;
    std::vector<double> mu,sigma;
};
class NaiveBayesClassifier : public Classifier{
using Dataset = std::vector<Sample>;
protected:
    std::vector<BasicBayesParaList> para;
    virtual bool CalcClassProb(float* prob) = 0;
    double CalculateClassProbability(unsigned int classID,const dataVec& x);
    void trainBayes(const Dataset& dataset,const float* classProbs);
public:
    NaiveBayesClassifier(){this->classifierName = "naiveBayes";};
    ~NaiveBayesClassifier(){}
    int Predict(const dataVec& x) override;
    virtual void Trian(const Dataset& dataset) override;
};
class FisherClassifier : public Classifier{
using Dataset = std::vector<Sample>;
protected:
    std::vector<dataVec> mu;
    dataVec signal,projMat;
    void CalcSwSb(float** Sw,float** Sb,const std::vector<Sample>& samples);
public:
    FisherClassifier() {this->classifierName = "fisher";}
    ~FisherClassifier(){}
    virtual void Train(const Dataset& dataset) override;
    int Predict(const dataVec& x) override;
};
class SVMClassifier : public Classifier{
using Dataset = std::vector<Sample>;
protected:
    class OVOSVM {
        double learningRate,bias,limit;
        int maxIter;
        std::vector<dataVec> supportVectors;
        std::vector<int> supportLabels;
        std::vector<double> supportAlpha;
        dataVec weights;
        int positiveClass,negetiveClass;
        static constexpr double eps = 1e-6;
        double dot(const dataVec& x,const dataVec& y) {
            double result = 0.0;
            for (size_t i = 0; i < x.size(); i++)
                result += x[i] * y[i];
            return result;
        }
    public:
        OVOSVM(int pos,int neg,double limit = 0.01,double learningRate = 0.001, int maxIter = 1000)
        : learningRate(learningRate),maxIter(maxIter),positiveClass(pos),negetiveClass(neg) {}
        void train(const Dataset& dataset,const std::vector<int>& index, const std::vector<int>& y);
        bool predict(const dataVec& sample){return (dot(sample,weights) + bias)>0;}
        int getPositiveClass() const{ return positiveClass; }
        int getNegetiveClass() const{ return negetiveClass; }
    };
    std::vector<std::unique_ptr<OVOSVM>> classifiers;
public:
    SVMClassifier(){this->classifierName = "svm";}
    ~SVMClassifier(){}
    virtual void Train(const Dataset& dataset) override;
    int Predict(const dataVec& x) override;
};
class BPClassifier : public Classifier{
using Dataset = std::vector<Sample>;
protected:
    int classNum,hiddenSize;
    std::vector<dataVec> weightsInput2Hidden,weightsHidden2Output,deltaWeightsInput2Hidden,deltaWeightsHidden2Output;
    double learningRate,momentum;
    float activation(float x) {return 1.0 / (1.0 + exp(-x));}
    void initWeights();
    void forwardFeed(const dataVec& inputs,dataVec& neuronHidden,dataVec& neuronOutput);
    void backwardFeed(unsigned int loc,const dataVec& neuronInput,const dataVec& neuronHidden,const dataVec& neuronOutput);
public:
    BPClassifier(int hiddensize = 20, double rate = 0.4, double mom = 0.8):classNum(0),hiddenSize(hiddensize),learningRate(rate),momentum(mom) {this->classifierName = "bp";}
    ~BPClassifier(){}
    virtual void Train(const Dataset& dataset) override;
    int Predict(const dataVec& x) override;
};

class RandomForestClassifier : public Classifier{
using Dataset = std::vector<Sample>;
using ProbClass = unordered_map<int,double>;
protected:
    class DecisionTree {
    private:
        struct Node {
            int featureIndex;
            std::shared_ptr<Node> left;
            std::shared_ptr<Node> right;
            float threshold;
            ProbClass prob;
            bool isLeaf;
            Node(bool isLeaf,int featureIndex = -1, float threshold = 0,std::shared_ptr<Node> l = nullptr,std::shared_ptr<Node> r = nullptr):
                isLeaf(isLeaf),left(l),right(r),featureIndex(featureIndex),threshold(threshold) {}
        };
        std::shared_ptr<Node> root;
        int featureNum;
        int maxDepth;
        int minSamplesSplit,minSamplesLeaf;
        int maxFeature(int num) {return int(std::sqrt(num));}
        double computeGiniIndex(const Dataset& dataset,std::vector<pair<int, double>> samplesFeaturesVec,size_t splitIndex);
        void splitSamplesVec(const Dataset &dataset,const std::vector<int> &dataIndex,
            int &featureIndex, double &threshold,
                             std::vector<int> &leftDataIndex,std::vector<int> &rightDataIndex);
        void sortByFeatures(const Dataset& dataset,int featureIndex,std::vector<pair<int, double>>& samplesFeaturesVec);
        ProbClass computeTargetProb(const Dataset &dataset,const std::vector<int> &dataIndex);
        void chooseBestSplitFeatures(const Dataset &dataset,const std::vector<int> &dataIndex,int& featureIndex,double& threshold);
        std::shared_ptr<Node> constructNode(const Dataset &dataset,std::vector<int> dataIndex,int depth);

    public:
        DecisionTree(int featureNum,int maxDepth,int minSamplesSplit,int minSamplesLeaf)
            :featureNum(featureNum),maxDepth(maxDepth),minSamplesSplit(minSamplesSplit),minSamplesLeaf(minSamplesLeaf){};
        void train(const Dataset &dataset);
        ProbClass predict(const dataVec& x);
    };
    using DecisionTreeList = std::vector<std::unique_ptr<DecisionTree>>;
    DecisionTreeList decisionTrees;
    int nEstimators,eachTreeSamplesNum;
    int maxDepth,minSamplesSplit,minSamplesLeaf;
    void Bootstrapping(const Dataset& rawdataset, Dataset& bootstrapped);
public:
    RandomForestClassifier(int nEstimators,int maxDepth, int minSamplesSplit, int minSamplesLeaf, int eachTreeSamplesNum)
     : nEstimators(nEstimators),maxDepth(maxDepth),eachTreeSamplesNum(eachTreeSamplesNum),
        minSamplesSplit(minSamplesSplit),minSamplesLeaf(minSamplesLeaf) {
        decisionTrees.reserve(nEstimators);
        this->classifierName = "rf";
    }
    ~RandomForestClassifier(){}
    virtual void Train(const Dataset& dataset) override;
    int Predict(const dataVec& x) override;
};
#endif
