//
//  classifybase.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/16/24.
//

#include <cmath>
#include "classifybase.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../OpenGL/window.hpp"
#include "../OpenGL/commander.hpp"

void ClassMapper::generateRandomColorMap(int num){
    totalNum = num;
    for (int i = 0; i < num; i++){
        std::string name = "class " + std::to_string(i + 1);
        nameMap.push_back(name.c_str());
    }
    for (int i = 0; i < num; i++){
        unsigned char r = static_cast<float>(rand() % 100) / 100 * 255;
        unsigned char g = static_cast<float>(rand() % 100) / 100 * 255;
        unsigned char b = static_cast<float>(rand() % 100) / 100 * 255;
        colorMap.push_back(glm::vec3(r,g,b));
    }
}
void ClassMapper::readMapper(const std::vector<ROIcollection::ROIobject>& roiCollection){
    totalNum = static_cast<int>(roiCollection.size());
    for (std::vector<ROIcollection::ROIobject>::const_iterator collection = roiCollection.begin(); collection != roiCollection.end(); collection++){
        nameMap.push_back(collection->name);
        colorMap.push_back(collection->color);
    }
}
float Accuracy::getComprehensiveAccuracy(){
    float accuracy = 0.0f;
    for (std::vector<float>::const_iterator it = f1.begin(); it != f1.end(); it++)
        accuracy += *it;
    accuracy /= f1.size();
    return accuracy;
}
void Accuracy::PrintPrecision(){
    ClassMapper& classMapper = ClassMapper::getClassMap();
    const int n = classMapper.getTotalNum();
    ImGui::PushFont(gui::chineseFont);
    ImGui::Text("<混淆矩阵>");
    if (ImGui::BeginTable("##confus mat", n + 1,ImGuiTableFlags_Borders)){
        ImGui::TableSetupColumn("类型");
        for (int i = 0; i < n; i++)
            ImGui::TableSetupColumn(classMapper.getName(i).c_str());
        ImGui::TableHeadersRow();
        for (int i = 0; i < n; i++){
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s",classMapper.getName(i).c_str());
            for (int j = 0; j < n; j++){
                ImGui::TableNextColumn();
                ImGui::Text("%s",std::to_string(confuseMat[i][j]).c_str());
            }
        }
    }
    ImGui::EndTable();
    ImGui::Text("<Precision>");
    if (ImGui::BeginTable("##confus mat", n,ImGuiTableFlags_Borders)){
        for (int i = 0; i < n; i++)
            ImGui::TableSetupColumn(classMapper.getName(i).c_str());
        for (int i = 0; i < n; i++){
            ImGui::TableNextColumn();
            std::stringstream oss;
            oss<<std::setprecision(2)<<precision[i];
            ImGui::Text("%s",oss.str().c_str());
        }
    }
    ImGui::EndTable();
    ImGui::Text("<Recall>");
    if (ImGui::BeginTable("##confus mat", n,ImGuiTableFlags_Borders)){
        for (int i = 0; i < n; i++)
            ImGui::TableSetupColumn(classMapper.getName(i).c_str());
        for (int i = 0; i < n; i++){
            ImGui::TableNextColumn();
            std::stringstream oss;
            oss<<std::setprecision(2)<<recall[i];
            ImGui::Text("%s",oss.str().c_str());
        }
    }
    ImGui::EndTable();
    ImGui::Text("<F1>");
    if (ImGui::BeginTable("##confus mat", n,ImGuiTableFlags_Borders)){
        for (int i = 0; i < n; i++)
            ImGui::TableSetupColumn(classMapper.getName(i).c_str());
        for (int i = 0; i < n; i++){
            ImGui::TableNextColumn();
            std::stringstream oss;
            oss<<std::setprecision(2)<<f1[i];
            ImGui::Text("%s",oss.str().c_str());
        }
    }
    ImGui::EndTable();
    ImGui::PopFont();
}
void Classifier::Classify(const std::vector<Band>& bands,unsigned char* classified){
    ClassMapper& classMapper = ClassMapper::getClassMap();
    int height = bands[0].value->height, width = bands[0].value->width;
    for (int y = 0; y < height; y += margin)
        for (int x = 0; x < width; x += margin){
            dataVec dataFeatures;
            bool blankPixel = false;
            for (std::vector<Band>::const_iterator band = bands.begin(); band != bands.end(); band++){
                float feature = 0;
                int count = 0;
                for (int i = y; i < y + margin; i++)
                    for (int j = x; j < x + margin; j++){
                        if (band->value->rawData[i][j] == 0)
                            continue;
                        feature += band->value->rawData[i][j];
                        ++count;
                    }
                if (count == 0){
                    blankPixel = true;
                    for (int i = y; i < y + margin; i++)
                        for (int j = x; j < x + margin; j++){
                            int loc = i * width + j;
                            classified[loc * 3 + 0] = classMapper.blankColor.r;
                            classified[loc * 3 + 1] = classMapper.blankColor.g;
                            classified[loc * 3 + 2] = classMapper.blankColor.b;
                        }
                    break;
                }
                dataFeatures.push_back(feature / count);
            }
            if (blankPixel)
                continue;
            int label = Predict(dataFeatures);
            glm::vec3 color = classMapper.getColor(label);
            for (int i = y; i < y + margin; i++)
                for (int j = x; j < x + margin; j++){
                    int loc = i * width + j;
                    classified[loc * 3 + 0] = color.r;
                    classified[loc * 3 + 1] = color.g;
                    classified[loc * 3 + 2] = color.b;
                }
        }
}
void Classifier::Examine(const Dataset& samples){
    ClassMapper& classMapper = ClassMapper::getClassMap();
    const int classNum = classMapper.getTotalNum();
    accuracy.confuseMat.assign(classNum,std::vector<int>(classNum,0));
    std::vector<int> TP(classNum,0),FP(classNum,0),FN(classNum,0);
    for (typename Dataset::const_iterator it = samples.begin(); it != samples.end(); it++){
        if (it->isTrainSample())
            continue;
        int trueLabel = it->getLabel(), predictLabel = Predict(it->getFeatures());
        ++accuracy.confuseMat[trueLabel][predictLabel];
        if (predictLabel == trueLabel)
            ++TP[predictLabel];
        else{
            ++FP[predictLabel];
            ++FN[trueLabel];
        }
    }
    for (int i = 0; i < classNum; i++){
        float precision = TP[i] / (TP[i] + FP[i]),recall = TP[i] / (TP[i] + FN[i]);
        accuracy.precision[i] = precision;
        accuracy.recall[i] = recall;
        accuracy.f1[i] = 2 * precision * recall / (precision + recall);
    }
}
void ScanLineEdgeConstruct(std::vector<ScanLineEdge>& position,std::shared_ptr<ROI> part,OGRCoordinateTransformation *transformation,int pixelSize){
    std::vector<glm::vec2> sortedPos;
    part->getSortedVertex(sortedPos,transformation);
    
    int minY = static_cast<int>(sortedPos.front().y), maxY = static_cast<int>(sortedPos.back().y);
    int edge1X = sortedPos[0].x,edge2X = sortedPos[0].x;
    size_t vecPoint = 0;
    position.push_back(ScanLineEdge(minY, sortedPos[0].x,  sortedPos[0].x));
    std::pair<double,int> edge[2]; //slope + termY
    if (abs(sortedPos[vecPoint + 1].y - sortedPos[vecPoint].y) < pixelSize){
        edge[0] = std::make_pair(0,sortedPos[vecPoint + 1].y);
        edge1X = sortedPos[vecPoint + 1].x;
    }else
        edge[0] = std::make_pair((sortedPos[vecPoint + 1].x - sortedPos[vecPoint].x) / (sortedPos[vecPoint + 1].y - sortedPos[vecPoint].y),sortedPos[vecPoint + 1].y);
    if (abs(sortedPos[vecPoint + 2].y - sortedPos[vecPoint].y) < pixelSize){
        edge[1] = std::make_pair(0,sortedPos[vecPoint + 2].y);
        edge2X = sortedPos[vecPoint + 2].x;
    }else
        edge[1] = std::make_pair((sortedPos[vecPoint + 2].x - sortedPos[vecPoint].x) / (sortedPos[vecPoint + 2].y - sortedPos[vecPoint].y),sortedPos[vecPoint + 2].y);
    int y = minY + pixelSize;
    while (vecPoint + 3 <= sortedPos.size()){
        edge1X += edge[0].first * pixelSize;
        edge2X += edge[1].first * pixelSize;
        if (edge1X <= edge2X)
            position.push_back(ScanLineEdge(y, edge1X, edge2X));
        else
            position.push_back(ScanLineEdge(y, edge2X, edge1X));
        y += pixelSize;
        if (y > edge[0].second){
            ++vecPoint;
            if (vecPoint + 3 > sortedPos.size())
                break;
            if (abs(sortedPos[vecPoint + 2].y - sortedPos[vecPoint].y) < pixelSize){
                edge[0] = std::make_pair(0, sortedPos[vecPoint + 2].y);
                edge1X = sortedPos[vecPoint + 2].x;
            }else
                edge[0] = std::make_pair((sortedPos[vecPoint + 2].x - sortedPos[vecPoint].x) / (sortedPos[vecPoint + 2].y - sortedPos[vecPoint].y),sortedPos[vecPoint + 2].y);
        }
        if (y > edge[1].second){
            ++vecPoint;
            if (vecPoint + 3 > sortedPos.size())
                break;
            if (abs(sortedPos[vecPoint + 2].y - sortedPos[vecPoint].y) < pixelSize){
                edge[1] = std::make_pair(0, sortedPos[vecPoint + 2].y);
                edge2X = sortedPos[vecPoint + 2].x;
            }else
                edge[1] = std::make_pair((sortedPos[vecPoint + 2].x - sortedPos[vecPoint].x) / (sortedPos[vecPoint + 2].y - sortedPos[vecPoint].y),sortedPos[vecPoint + 2].y);
        }
    }
    for (; y < maxY; y += pixelSize){
        edge1X += edge[0].first * pixelSize;
        edge2X += edge[1].first * pixelSize;
        if (edge1X <= edge2X)
            position.push_back(ScanLineEdge(y, edge1X, edge2X));
        else
            position.push_back(ScanLineEdge(y, edge2X, edge1X));
    }
}
