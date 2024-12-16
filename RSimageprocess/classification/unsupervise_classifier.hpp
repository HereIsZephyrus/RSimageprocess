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

enum class UnsuperviseClassifierType{
    isodata,
    kmean
};
/*
class ISODATA{
    double epsilon,splitThreshold;
    int maxIter,minSamples,clusterNum;
    int width,height;
    std::vector<int> centersID;
    std::shared_ptr<std::vector<Band>> bandsPtr;
    double euclideanDistance(const std::vector<double>& a, const std::vector<double>& b);
    std::vector<std::vector<double>> updateCenters(const std::vector<std::vector<double>>& data, const std::vector<int>& labels, int numClusters);
    std::vector<double> calculateVariance(const std::vector<std::vector<double>>& data, const std::vector<int>& labels, const std::vector<std::vector<double>>& centers, int numClusters);
public:
    ISODATA(int CLUSTER_NUM,int MAX_ITER = 100,int MIN_SAMPLES = 5,double EPSILON = 1e-6,double SPLIT_THRESHOLD = 1.0);
    void bindDataset(const std::vector<Band>& bands);
    void isodata(const std::vector<std::vector<double>>& data, int initialClusters, int& finalClusters, std::vector<int>& labels) {
        size_t n = data.size();
        size_t dim = data[0].size();

        std::vector<std::vector<double>> centers(initialClusters, std::vector<double>(dim));
        labels.assign(n, -1);

        // 随机初始化类中心
        std::srand(std::time(0));
        for (int k = 0; k < initialClusters; ++k) {
            centers[k] = data[std::rand() % n];
        }

        int currentClusters = initialClusters;
        for (int iter = 0; iter < MAX_ITER; ++iter) {
            // 分配样本到最近的类
            bool converged = true;
            for (size_t i = 0; i < n; ++i) {
                double minDist = std::numeric_limits<double>::max();
                int bestCluster = -1;
                for (int k = 0; k < currentClusters; ++k) {
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

            // 更新类中心
            centers = updateCenters(data, labels, currentClusters);

            // 检查收敛条件
            if (converged) {
                break;
            }

            // 计算类内方差
            std::vector<double> variance = calculateVariance(data, labels, centers, currentClusters);

            // 分裂类
            for (int k = 0; k < currentClusters; ++k) {
                if (variance[k] > SPLIT_THRESHOLD) {
                    // 分裂为两个新类
                    std::vector<double> newCenter = centers[k];
                    for (size_t j = 0; j < dim; ++j) {
                        newCenter[j] += 0.5; // 偏移一点形成新类
                    }
                    centers.push_back(newCenter);
                    ++currentClusters;
                }
            }

            // 合并类
            for (int k = 0; k < currentClusters; ++k) {
                for (int l = k + 1; l < currentClusters; ++l) {
                    if (euclideanDistance(centers[k], centers[l]) < EPSILON) {
                        // 合并类 k 和 l
                        for (size_t j = 0; j < dim; ++j) {
                            centers[k][j] = (centers[k][j] + centers[l][j]) / 2.0;
                        }
                        centers.erase(centers.begin() + l);
                        --currentClusters;
                        --l;
                    }
                }
            }
        }

        finalClusters = currentClusters;
    }

};
*/
#endif /* unsupervise_classifier_hpp */
