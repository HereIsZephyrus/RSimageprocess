//
//  classifybase.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/16/24.
//

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
        float r = static_cast<float>(rand() % 100) / 100;
        float g = static_cast<float>(rand() % 100) / 100;
        float b = static_cast<float>(rand() % 100) / 100;
        colorMap.push_back(glm::vec3(r,g,b));
    }
    //ImGui::ColorEdit3(std::string("##Color" + nameMap[0]).c_str(), (float*)&colorMap[0]);
    //for (int i = 1; i < num; i++){
    //    ImGui::SameLine();
    //    ImGui::ColorEdit3(std::string("##Color" + nameMap[i]).c_str(), (float*)&colorMap[i]);
    //}
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
    classified = new unsigned char[height * width * 3];
    for (int y = 0; y < height; y += margin)
        for (int x = 0; x < width; x += margin){
            dataVec dataFeatures;
            bool blankPixel = false;
            for (std::vector<Band>::const_iterator band = bands.begin(); band != bands.end(); band++){
                float feature = 0;
                int count = 0;
                for (int i = y; i < y + margin; i++)
                    for (int j = x; i < x + margin; j++){
                        if (band->value->rawData[i][j] == 0)
                            continue;
                        feature += band->value->rawData[i][j];
                        ++count;
                    }
                if (count == 0){
                    blankPixel = true;
                    for (int i = y; i < y + margin; i++)
                        for (int j = x; i < x + margin; j++){
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
                for (int j = x; i < x + margin; j++){
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
