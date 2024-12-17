//
//  classifier.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/16/24.
//

#include "supervise_classifier.hpp"

void FisherClassifier::CalcSwSb(float** Sw,float** Sb,const std::vector<Sample>& samples){
    const int classNum = ClassMapper::getClassMap().getTotalNum();
    dataVec featureAvg(this->featureNum, 0.0f);
    dataVec classifiedFeaturesAvg[classNum];
    for (int i = 0; i < classNum; i++)
        classifiedFeaturesAvg[i].assign(this->featureNum,0.0);
    std::vector<size_t> classRecordNum(classNum,0);
    int total = 0;
    for (Dataset::const_iterator it = samples.begin(); it != samples.end(); it++){
        if (!it->isTrainSample())
            continue;
        unsigned int label = static_cast<unsigned int>(it->getLabel());
        const dataVec& sampleFeature = it->getFeatures();
        for (unsigned int i = 0; i < this->featureNum; i++)
            classifiedFeaturesAvg[label][i] += sampleFeature[i];
        ++classRecordNum[label];
        ++total;
    }
    for (unsigned int i = 0; i < classNum; i++)
        for (unsigned int j = 0; j < this->featureNum; j++){
            featureAvg[j] += classifiedFeaturesAvg[i][j];
            classifiedFeaturesAvg[i][j] /= classRecordNum[i];
        }
    for (unsigned int j = 0; j < this->featureNum; j++)
        featureAvg[j] /= total;
    for (unsigned int i = 0; i < classNum; i++)
        mu.push_back(classifiedFeaturesAvg[i]);
    for (Dataset::const_iterator it = samples.begin(); it != samples.end(); it++){
        if (!it->isTrainSample())
            continue;
        unsigned int label = it->getLabel();
        const dataVec& sampleFeature = it->getFeatures();
        dataVec pd = sampleFeature;
        for (int j = 0; j < this->featureNum; j++)
            pd[j] -= classifiedFeaturesAvg[label][j];
        for (int j = 0; j < this->featureNum; ++j)
            for (int k = 0; k < this->featureNum; ++k)
                Sw[j][k] += pd[j] * pd[k];
    }
    for (int i = 0; i < classNum; i++) {
        dataVec pd(this->featureNum, 0.0f);
        for (int j = 0; j < this->featureNum; j++)
            pd[j] = classifiedFeaturesAvg[i][j] - featureAvg[j];
        for (int j = 0; j < this->featureNum; j++)
            for (int k = 0; k < this->featureNum; k++)
                Sb[j][k] += classRecordNum[i] * pd[j] * pd[k];
    }
}
void FisherClassifier::Train(const Dataset& dataset){
    using fMat = float**;
    this->featureNum = dataset[0].getFeatures().size();
    fMat SwMat,SbMat;
    SwMat = new float*[this->featureNum];
    SbMat = new float*[this->featureNum];
    for (size_t i = 0; i < this->featureNum; i++){
        SwMat[i] = new float[this->featureNum];
        SbMat[i] = new float[this->featureNum];
        for (size_t j = 0; j < this->featureNum; j++){
            SwMat[i][j] = 0.0f;
            SbMat[i][j] = 0.0f;
        }
    }
    CalcSwSb(SwMat,SbMat,dataset);
    MatrixXf Sw(this->featureNum,this->featureNum),Sb(this->featureNum,this->featureNum);
    for (size_t i = 0; i < this->featureNum; i++)
        for (size_t j = 0; j < this->featureNum; j++){
            Sw(i,j) = SwMat[i][j];
            Sb(i,j) = SbMat[i][j];
        }
    for (size_t i = 0; i < this->featureNum; i++){
        delete[] SwMat[i];
        delete[] SbMat[i];
    }
    delete[] SwMat;
    delete[] SbMat;
    SelfAdjointEigenSolver<MatrixXf> eig(Sw.completeOrthogonalDecomposition().pseudoInverse() * Sb);
    const int classNum = ClassMapper::getClassMap().getTotalNum();
    MatrixXf projectionMatrix = eig.eigenvectors().rightCols(1);
    for (size_t j = 0; j < this->featureNum; j++){
        projMat.push_back(projectionMatrix(j));
    }
    for (int i = 0; i < classNum; i++){
        float calcmean = 0;
        for (size_t j = 0; j < this->featureNum; j++)
            calcmean += projectionMatrix(j) * mu[i][j];
        signal.push_back(calcmean);
        //std::cout<<signal[i]<<std::endl;
    }
    return;
}
int FisherClassifier::Predict(const dataVec& x){
    const int classNum = ClassMapper::getClassMap().getTotalNum();
    int resClass = 0;
    double projed = 0;
    for (unsigned int i = 0; i < this->featureNum; i++)
        projed += x[i] * projMat[i];
    double minDistance = 1e10;
    for (int classID = 0; classID < classNum; classID++){
        double distance = (projed - signal[classID])*(projed - signal[classID]);
        if (distance < minDistance) {
            minDistance = distance;
            resClass = classID;
        }
    }
    return resClass;
}
void SVMClassifier::OVOSVM::train(const Dataset& dataset,const std::vector<int>& index, const std::vector<int>& y) {
    int sampleNum = static_cast<int>(index.size());
    int featureNum = static_cast<int>(dataset[index[0]].getFeatures().size());
    weights.assign(featureNum, 0.0);
    double beta = 1.0;
    bool notMargined = true;
    dataVec alpha;
    alpha.assign(sampleNum, 0.0);
    for (int iter = 0; iter < maxIter; ++iter) {
        notMargined = false;
        double error = 0;
        for (int i = 0; i < sampleNum; i++) {
            double item1 = 0.0;
            for (int j = 0; j < sampleNum; j++)
                item1 += alpha[j] * (double)y[i] * (double)y[j] * dot(dataset[index[i]].getFeatures(), dataset[index[j]].getFeatures());
            double item2 = 0.0;
            for (int j = 0; j < sampleNum; j++)
                item2 += alpha[j] * (double)y[i] * (double)y[j];
            double delta = 1.0 - item1 - beta * item2;
            alpha[i] += learningRate * delta;
            alpha[i] = std::max(alpha[i],0.0f);
            if (std::abs(delta) > limit){
                notMargined = true;
                error += std::abs(delta) - limit;
            }
        }
        double item3 = 0.0;
        for (int i = 0; i < sampleNum; i++)
            item3 += alpha[i] * (double)y[i];
        beta += item3 * item3 / 2.0;
        if (!notMargined)   break;
    }
    for (int i = 0; i < sampleNum; i++){
        if (alpha[i] > eps){
            supportVectors.push_back(dataset[index[i]].getFeatures());
            supportLabels.push_back(y[i]);
            supportAlpha.push_back(alpha[i]);
        }
    }
    weights.assign(featureNum,0.0);
    for (int j = 0; j < featureNum; j++)
        for (size_t i = 0; i < supportVectors.size(); i++)
            weights[j] += supportAlpha[i] * supportLabels[i] * supportVectors[i][j];
    bias = 0.0;
    for (size_t i = 0; i < supportVectors.size(); i++)
        bias += supportLabels[i] - dot(weights, supportVectors[i]);
    bias /= static_cast<double>(supportVectors.size());
}
void SVMClassifier::Train(const Dataset& dataset) {
    this->featureNum = dataset[0].getFeatures().size();
    const int classNum = ClassMapper::getClassMap().getTotalNum();
    std::vector<int> classCount[classNum];
    for (size_t i = 0; i < dataset.size(); i++){
        const Sample& sample = dataset[i];
        if (!sample.isTrainSample())
            continue;
        unsigned int classID = static_cast<unsigned int>(sample.getLabel());
        classCount[classID].push_back(static_cast<int>(i));
    }
    for (unsigned int i = 0; i < classNum; i++)
        for (unsigned int j = i+1; j < classNum; j++){
            std::vector<int> classPN = classCount[i];
            std::vector<int> classLabeli,classLabelj;
            classLabeli.assign(classCount[i].size(),1);
            classLabelj.assign(classCount[j].size(),-1);
            classLabeli.insert(classLabeli.end(), classLabelj.begin(), classLabelj.end());
            classPN.insert(classPN.end(), classCount[j].begin(), classCount[j].end());
            std::unique_ptr<OVOSVM> classifier = std::make_unique<OVOSVM>(i,j);
            classifier->train(dataset,classPN,classLabeli);
            classifiers.push_back(std::move(classifier));
        }
};
int SVMClassifier::Predict(const dataVec& x) {
    const int classNum = ClassMapper::getClassMap().getTotalNum();
    std::vector<unsigned int> classVote(classNum);
    classVote.assign(classNum,0);
    for (std::vector<std::unique_ptr<OVOSVM>>::iterator it = classifiers.begin(); it != classifiers.end(); it++){
        if ((*it)->getNegetiveClass() > classNum){
            if ((*it)->predict(x))
                return (*it)->getPositiveClass();
        }else{
            if ((*it)->predict(x))
                ++classVote[static_cast<unsigned int>((*it)->getPositiveClass())];
            else
                ++classVote[static_cast<unsigned int>((*it)->getNegetiveClass())];
        }
    }
    int maxVote = 0;
    int resClass = 0;
    for (std::vector<unsigned int>::const_iterator it = classVote.begin(); it != classVote.end(); it++)
        if (*it > maxVote){
            maxVote = *it;
            resClass = static_cast<int>(it - classVote.begin());
        }
    return resClass;
}
void BPClassifier::initWeights(){
    weightsInput2Hidden.assign(this->featureNum+1,dataVec(hiddenSize,0));
    weightsHidden2Output.assign(hiddenSize+1,dataVec(classNum,0));
    deltaWeightsInput2Hidden.assign(this->featureNum+1,dataVec(hiddenSize,0));
    deltaWeightsHidden2Output.assign(hiddenSize+1,dataVec(classNum,0));
    double rangeHidden = 1/sqrt((double)this->featureNum);
    double rangeOutput = 1/sqrt((double)hiddenSize);
    for (int i = 0; i <= this->featureNum; i++)
        for (int j = 0; j < hiddenSize; j++)
            weightsInput2Hidden[i][j] = (((double)(rand() % 100 + 1))/100.0) * 2.0 * rangeHidden - rangeHidden;
    for (int j = 0; j <= hiddenSize; j++)
        for (int k = 0; k < classNum; k++)
            weightsHidden2Output[j][k] = (((double)(rand() % 100 + 1))/100.0) * 2.0 * rangeOutput - rangeOutput;
}
void BPClassifier::forwardFeed(const dataVec& inputs,dataVec& neuronHidden,dataVec& neuronOutput) {
    dataVec neuronInput = inputs;
    neuronInput.push_back(-1); // bias neuronn
    neuronHidden.assign(hiddenSize,0);
    neuronHidden.push_back(-1); // bias neuronn
    neuronOutput.assign(classNum,0);
    for (int j = 0; j < hiddenSize; j++){
        for (int i = 0; i <= this->featureNum; i++)
            neuronHidden[j] += neuronInput[i] * weightsInput2Hidden[i][j];
        neuronHidden[j] = activation(neuronHidden[j]);
    }
    for (int k = 0; k < classNum; k++){
        for (int j = 0; j <= hiddenSize; j++)
            neuronOutput[k] += neuronHidden[j] * weightsHidden2Output[j][k];
        neuronOutput[k] = activation(neuronOutput[k]);
    }
}
void BPClassifier::backwardFeed(unsigned int loc,const dataVec& neuronInput,const dataVec& neuronHidden,const dataVec& neuronOutput) {
    dataVec idealOutput(classNum,0.0f);
    dataVec errorOutput(classNum,0.0f);
    dataVec errorHidden(hiddenSize+1,0.0f);
    idealOutput[loc] = 1.0f;
    for (int k = 0; k < classNum; k++)
        errorOutput[k] = neuronOutput[k] * (1 - neuronOutput[k]) * (idealOutput[k] - neuronOutput[k]);
    for (int j = 0; j <= hiddenSize; j++){
        int sum = 0;
        for (int k = 0; k < classNum; k++){
            sum += weightsHidden2Output[j][k] * errorOutput[k];
            deltaWeightsHidden2Output[j][k] = learningRate * neuronHidden[j] * errorOutput[k] + momentum * deltaWeightsHidden2Output[j][k];
            weightsHidden2Output[j][k] += deltaWeightsHidden2Output[j][k];
        }
        errorHidden[j] = neuronHidden[j] * (1 - neuronHidden[j]) * sum;
    }
    for (int i = 0; i <= this->featureNum; i++)
        for (int j = 0; j < hiddenSize; j++){
            deltaWeightsInput2Hidden[i][j] = learningRate * neuronInput[i] * errorHidden[j] + momentum * deltaWeightsInput2Hidden[i][j];
            weightsInput2Hidden[i][j] += deltaWeightsInput2Hidden[i][j];
        }
}
void BPClassifier::Train(const Dataset& dataset) {
    this->featureNum = dataset[0].getFeatures().size();
    const int classNum = ClassMapper::getClassMap().getTotalNum();
    initWeights();
    int maxiter = 100;
    while(maxiter--){
        double TMSE = 0,Tacc = 0;
        int total = 0;
        for (typename Dataset::const_iterator data = dataset.begin(); data != dataset.end(); data++){
            if (!data->isTrainSample())
                continue;
            ++total;
            int label = data->getLabel(),resClass;
            unsigned int uLabel = static_cast<unsigned int>(label);
            double maxVal = -1.0;
            dataVec hidden,output;
            forwardFeed(data->getFeatures(),hidden,output);
            TMSE += (1.0 - output[uLabel]) * (1.0 - output[uLabel]);
            for (int k = 0; k < classNum; k++)
                if (output[k] > maxVal){
                    maxVal = output[k];
                    resClass = k;
                }
            if (label == resClass)  Tacc += 1.0;
            backwardFeed(uLabel,data->getFeatures(),hidden,output);
        }
        TMSE = TMSE / (double)total;
        Tacc = Tacc / (double)total * 100.0;
        if (TMSE < 0.02 || Tacc > 98)
            break;
    }
}
int BPClassifier::Predict(const dataVec& x) {
    dataVec hidden,actived;
    forwardFeed(x,hidden,actived);
    int resClass = 0;
    float maxLight = 0.0f;
    for (int i = 0; i < classNum; i++)
        if (actived[i] > maxLight){
            maxLight = actived[i];
            resClass = i;
        }
    return resClass;
}
double RandomForestClassifier::DecisionTree::computeGiniIndex(const Dataset& dataset,std::vector<pair<int, double>> samplesFeaturesVec,size_t splitIndex) {
    unordered_map<int,int> leftCounter,rightCounter;
    size_t totalSize = samplesFeaturesVec.size();
    for (size_t index = 0; index < splitIndex; index++)
        leftCounter[dataset[samplesFeaturesVec[index].first].getLabel()]++;
    for (size_t index = splitIndex; index < totalSize; index++)
        rightCounter[dataset[samplesFeaturesVec[index].first].getLabel()]++;
    double leftGini = 0.0,rightGini = 0.0;
    for (typename unordered_map<int,int>::const_iterator label = leftCounter.begin(); label != leftCounter.end(); label++)
        leftGini += (double)(label->second) * (label->second) / (double)(splitIndex * splitIndex);
    for (typename unordered_map<int,int>::const_iterator label = rightCounter.begin(); label != rightCounter.end(); label++)
        rightGini += (double)(label->second) * (label->second) / (double)((totalSize - splitIndex) * (totalSize - splitIndex));
    return (double)(splitIndex/totalSize) * leftGini + (double)(1 - splitIndex/totalSize) * rightGini;
}
void RandomForestClassifier::DecisionTree::splitSamplesVec(const Dataset &dataset,const std::vector<int> &dataIndex,
    int &featureIndex, double &threshold,
    std::vector<int> &leftDataIndex,std::vector<int> &rightDataIndex){
    leftDataIndex.clear();
    rightDataIndex.clear();
    for (std::vector<int>::const_iterator index = dataIndex.begin(); index != dataIndex.end(); index++){
        if (dataset[*index].getFeatures()[featureIndex] <= threshold)
            leftDataIndex.push_back(*index);
        else
            rightDataIndex.push_back(*index);
    }
}
void RandomForestClassifier::DecisionTree::sortByFeatures(const Dataset& dataset,int featureIndex,std::vector<pair<int, double>>& samplesFeaturesVec) {
    for (std::vector<pair<int, double>>::iterator sample = samplesFeaturesVec.begin(); sample != samplesFeaturesVec.end(); sample++)
        sample->second = dataset[sample->first].getFeatures()[featureIndex];
    sort(samplesFeaturesVec.begin(), samplesFeaturesVec.end(),
    [](pair<int,double>& a, pair<int, double>& b) {return a.second < b.second;});
}
unordered_map<int,double> RandomForestClassifier::DecisionTree::computeTargetProb(const Dataset &dataset,const std::vector<int> &dataIndex){
    ProbClass resProb;
    for (std::vector<int>::const_iterator index = dataIndex.begin(); index != dataIndex.end(); index++)
        resProb[dataset[*index].getLabel()]++;
    for (typename ProbClass::iterator prob = resProb.begin(); prob != resProb.end(); prob++)
        prob->second /= dataIndex.size();
    return resProb;
}
void RandomForestClassifier::DecisionTree::chooseBestSplitFeatures(const Dataset &dataset,const std::vector<int> &dataIndex,int& featureIndex,double& threshold){
    std::set<int> featureBucket;
    std::vector<int> featuresVec;
    int maxFeatureNum = maxFeature(featureNum);
    while (featureBucket.size() < maxFeatureNum){
        int index = rand() % featureNum;
        featureBucket.insert(index);
    }
    for (std::set<int> ::const_iterator feature = featureBucket.begin(); feature != featureBucket.end(); feature++)
        featuresVec.push_back(*feature);
    int bestFeatureIndex = featuresVec[0];
    double minValue = 1e6, bestThreshold = 0;
    std::vector<pair<int, double>> samplesFeaturesVec(dataIndex.size());
    for (size_t i = 0; i < dataIndex.size(); i++)
        samplesFeaturesVec[i] = std::make_pair(dataIndex[i],0);
    for (std::vector<int>::const_iterator feature = featuresVec.begin(); feature != featuresVec.end(); feature++) {
        sortByFeatures(dataset,*feature,samplesFeaturesVec);
        int index = 0;
        for (size_t index = 0 ; index < samplesFeaturesVec.size(); index++){
            double value = computeGiniIndex(dataset,samplesFeaturesVec,index);
            if (value <= minValue) {
                minValue = value;
                bestThreshold = samplesFeaturesVec[index].second;
                bestFeatureIndex = *feature;
            }
        }
    }
    featureIndex = bestFeatureIndex;
    threshold = bestThreshold;
}
std::shared_ptr<RandomForestClassifier::DecisionTree::Node> RandomForestClassifier::DecisionTree::constructNode(const Dataset &dataset,std::vector<int> dataIndex,int depth){
    ProbClass targetProb = computeTargetProb(dataset, dataIndex);
    bool pureNode = false;
    const double eps = 1e-6;
    for (typename ProbClass::iterator prob = targetProb.begin(); prob != targetProb.end(); prob++)
        if (std::abs(prob->second-1) < eps)
            pureNode = true;
    if (pureNode || depth >= maxDepth || dataIndex.size() < minSamplesSplit) {
        std::shared_ptr<Node> leaf = std::make_shared<Node>(true);
        leaf->prob = targetProb;
        return leaf;
    }
    int featureIndex;
    double threshold;
    std::vector<int> leftIndex, rightIndex;
    chooseBestSplitFeatures(dataset,dataIndex,featureIndex,threshold);
    splitSamplesVec(dataset,dataIndex,featureIndex,threshold,leftIndex,rightIndex);
    if ((leftIndex.size() < minSamplesLeaf) or (rightIndex.size() < minSamplesLeaf)) {
        std::shared_ptr<Node> node = std::make_shared<Node>(true,featureIndex,threshold);
        node->prob = targetProb;
        return node;
    } else
        return std::make_shared<Node>(false,featureIndex,threshold,
            constructNode(dataset,leftIndex,depth+1),constructNode(dataset,rightIndex,depth+1));
}
void RandomForestClassifier::DecisionTree::train(const std::vector<Sample> &dataset){
    std::vector<int> dataIndex;
    for (size_t i = 0; i < dataset.size(); i++)
        if (dataset[i].isTrainSample())
            dataIndex.push_back(static_cast<int>(i));
    root = constructNode(dataset,dataIndex,0);
}
unordered_map<int,double> RandomForestClassifier::DecisionTree::predict(const dataVec& x){
    std::shared_ptr<Node> node = root;
    while (!node->isLeaf) {
        if (x[node->featureIndex] <= node->threshold)
            node = node -> left;
        else
            node = node -> right;
    }
    return node->prob;
}
void RandomForestClassifier::Bootstrapping(const Dataset& rawdataset, Dataset& bootstrapped){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, static_cast<int>(rawdataset.size() - 1));
    for (int i = 0; i < eachTreeSamplesNum; i++){
        int index = dist(gen);
        bootstrapped.push_back(rawdataset[index]);
    }
}
void RandomForestClassifier::Train(const Dataset& dataset) {
    this->featureNum = dataset[0].getFeatures().size();
    eachTreeSamplesNum = std::min(eachTreeSamplesNum, static_cast<int>(dataset.size()));
    for (int i = 0; i < nEstimators; i++){
        std::unique_ptr<DecisionTree> tree = std::make_unique<DecisionTree>(this->featureNum,maxDepth,minSamplesSplit,minSamplesLeaf);
        Dataset bootstrap;
        Bootstrapping(dataset,bootstrap);
        tree->train(bootstrap);
        decisionTrees.push_back(std::move(tree));
    }
}
int RandomForestClassifier::Predict(const dataVec& x) {
    ProbClass votes;
    for (DecisionTreeList::iterator tree = decisionTrees.begin(); tree != decisionTrees.end(); tree++){
        ProbClass singleVote = (*tree)->predict(x);
        for (ProbClass::iterator prob = singleVote.begin(); prob != singleVote.end(); prob++)
            votes[prob->first] += prob->second;
    }
    return std::max_element(votes.begin(), votes.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; })->first;
}
