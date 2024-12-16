//
//  unsupervise_classifier.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/16/24.
//

#include "unsupervise_classifier.hpp"
#include <cstdlib>
#include <ctime>
/*
double ISODATA::euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++)
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    return std::sqrt(sum);
}
// 更新类中心
std::vector<std::vector<double>> ISODATA::updateCenters(const std::vector<std::vector<double>>& data, const std::vector<int>& labels, int numClusters) {
    size_t dim = data[0].size(); // 特征维度
    std::vector<std::vector<double>> centers(numClusters, std::vector<double>(dim, 0.0));
    std::vector<int> count(numClusters, 0);

    for (size_t i = 0; i < data.size(); ++i) {
        int label = labels[i];
        for (size_t j = 0; j < dim; ++j) {
            centers[label][j] += data[i][j];
        }
        ++count[label];
    }

    for (int k = 0; k < numClusters; ++k) {
        if (count[k] > 0) {
            for (size_t j = 0; j < dim; ++j) {
                centers[k][j] /= count[k];
            }
        }
    }
    return centers;
}
std::vector<double> ISODATA::calculateVariance(const std::vector<std::vector<double>>& data, const std::vector<int>& labels, const std::vector<std::vector<double>>& centers, int numClusters) {
    size_t dim = data[0].size();
    std::vector<double> variance(numClusters, 0.0);
    std::vector<int> count(numClusters, 0);

    for (size_t i = 0; i < data.size(); ++i) {
        int label = labels[i];
        variance[label] += euclideanDistance(data[i], centers[label]);
        ++count[label];
    }

    for (int k = 0; k < numClusters; ++k) {
        if (count[k] > 0) {
            variance[k] /= count[k];
        }
    }
    return variance;
}
ISODATA::ISODATA(int CLUSTER_NUM,int MAX_ITER,int MIN_SAMPLES,double EPSILON,double SPLIT_THRESHOLD):
clusterNum(CLUSTER_NUM),maxIter(MAX_ITER),minSamples(MIN_SAMPLES),epsilon(EPSILON),splitThreshold(SPLIT_THRESHOLD),bandsPtr(nullptr){
}
void ISODATA::bindDataset(const std::vector<Band>& bands){
    width = bands[0].value->width;
    height = bands[0].value->height;
    const int num = width * height;
    srand((unsigned int)time(NULL));
    centersID.clear();
    for (int k = 0; k < clusterNum; k++)
        centersID.push_back(rand()%num);
    bandsPtr = std::shared_ptr<std::vector<Band>>(bands);
}
*/
