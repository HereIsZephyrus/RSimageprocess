//
//  unsupervise_classifier.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/16/24.
//

#include "unsupervise_classifier.hpp"
#include <cstdlib>
#include <ctime>

double ISODATA::euclideanDistance(const dataVec& a, const dataVec& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++)
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    return std::sqrt(sum);
}
std::vector<dataVec> ISODATA::updateCenters(const std::vector<dataVec>& data, const std::vector<int>& labels, int numClusters) {
    std::vector<dataVec> centers(numClusters, dataVec(featureNum, 0.0));
    std::vector<int> count(numClusters, 0);

    for (size_t i = 0; i < data.size(); i++) {
        int label = labels[i];
        for (size_t j = 0; j < featureNum; j++)
            centers[label][j] += data[i][j];
        ++count[label];
    }

    for (int k = 0; k < numClusters; k++) {
        if (count[k] > 0)
            for (size_t j = 0; j < featureNum; j++)
                centers[k][j] /= count[k];
    }
    return centers;
}
dataVec ISODATA::calculateVariance(const std::vector<dataVec>& data, const std::vector<int>& labels, const std::vector<dataVec>& centers, int numClusters) {
    dataVec variance(numClusters, 0.0);
    std::vector<int> count(numClusters, 0);

    for (size_t i = 0; i < data.size(); i++) {
        int label = labels[i];
        variance[label] += euclideanDistance(data[i], centers[label]);
        ++count[label];
    }

    for (int k = 0; k < numClusters; k++)
        if (count[k] > 0)
            variance[k] /= count[k];
    return variance;
}
ISODATA::ISODATA(int INIT_CLUSTER_NUM,int MAX_ITER,int MIN_SAMPLES,double EPSILON,double SPLIT_THRESHOLD):
clusterNum(INIT_CLUSTER_NUM),maxIter(MAX_ITER),minSamples(MIN_SAMPLES),epsilon(EPSILON),splitThreshold(SPLIT_THRESHOLD),bandsPtr(nullptr){
    this->classifierName = "ISODATA";
}
void ISODATA::Classify(const std::vector<Band>& bands, bool toAverage, unsigned char* classified) {
    ClassMapper& classMapper = ClassMapper::getClassMap();
    featureNum = bands.size();
    int height = bands[0].value->height, width = bands[0].value->width;
    classified = new unsigned char[height * width * 3];

    std::vector<dataVec> centers(clusterNum, dataVec(featureNum));
    std::vector<dataVec> data;
    std::vector<int> labels;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            if (bands[0].value->rawData[y][x] == 0)
                continue;
            dataVec feature;
            for (std::vector<Band>::const_iterator band = bands.begin(); band != bands.end(); band ++){
                if (toAverage)
                    feature.push_back(static_cast<float>(band->value->average(y, x)));
                else
                    feature.push_back(static_cast<float>(band->value->strech(y, x)));
            }
            labels.push_back(-1);
            data.push_back(feature);
        }
    const int n = static_cast<int>(data.size());
    for (int k = 0; k < clusterNum; k++)
        centers[k] = data[rand() % n];

    int currentClusters = clusterNum;
    for (int iter = 0; iter < maxIter; iter++) {
        bool converged = true;
        for (size_t i = 0; i < n; i++) {
            double minDist = 1e12;
            int bestCluster = -1;
            for (int k = 0; k < currentClusters; k++) {
                double dist = euclideanDistance(data[i], centers[k]);
                if (dist < minDist) {
                    minDist = dist;
                    bestCluster = k;
                }
            }
            if (labels[i] != bestCluster) {
                labels[i] = bestCluster;
                converged = false;
            }
        }
        centers = updateCenters(data, labels, currentClusters);
        if (converged)
            break;
        dataVec variance = calculateVariance(data, labels, centers, currentClusters);
        int nextClusters = currentClusters;
        for (int k = 0; k < currentClusters; k++) {
            if (variance[k] > splitThreshold) {
                dataVec newCenter = centers[k];
                for (size_t j = 0; j < featureNum; j++)
                    newCenter[j] += 5000;
                centers.push_back(newCenter);
                ++nextClusters;
            }
        }
        currentClusters = nextClusters;
        for (int k = 0; k < currentClusters; k++)
            for (int l = k + 1; l < currentClusters; l++)
                if (euclideanDistance(centers[k], centers[l]) < epsilon) {
                    for (size_t j = 0; j < featureNum; j++)
                        centers[k][j] = (centers[k][j] + centers[l][j]) / 2.0;
                    centers.erase(centers.begin() + l);
                    --nextClusters;
                    --l;
                }
        
    }
    clusterNum = currentClusters;
    classMapper.generateRandomColorMap(clusterNum);
    int count = 0;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++){
            int loc = y * width + x;
            glm::vec3 useColor;
            if (bands[0].value->rawData[y][x] == 0)
                useColor = classMapper.blankColor;
            else
                useColor = classMapper.colorMap[labels[count++]];
            classified[loc * 3 + 0] = useColor.r;
            classified[loc * 3 + 0] = useColor.g;
            classified[loc * 3 + 0] = useColor.b;
        }
}

