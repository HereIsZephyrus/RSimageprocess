//
//  unsupervise_classifier.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/16/24.
//

#ifndef unsupervise_classifier_hpp
#define unsupervise_classifier_hpp
#include <vector>
#include "../OpenGL/graphing.hpp"
#include "classifybase.hpp"

class ISODATA{
    double epsilon,splitThreshold;
    int maxIter,minSamples,clusterNum;
    int width,height;
    std::vector<int> centersID;
    std::shared_ptr<std::vector<Band>> bandsPtr;
    size_t featureNum;
    std::string classifierName;
    double euclideanDistance(const dataVec& a, const dataVec& b);
    std::vector<dataVec> updateCenters(const std::vector<dataVec>& data, const std::vector<int>& labels, int numClusters);
    dataVec calculateVariance(const std::vector<dataVec>& data, const std::vector<int>& labels, const std::vector<dataVec>& centers, int numClusters);
public:
    ISODATA(int INIT_CLUSTER_NUM,int MAX_ITER = 10,int MIN_SAMPLES = 5,double EPSILON = 1e-6,double SPLIT_THRESHOLD = 1.0);
    void Classify(const std::vector<Band>& bands, bool toAverage, unsigned char* classified);
};
#endif /* unsupervise_classifier_hpp */
