//
//  interface.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/11/24.
//

#include "interface.hpp"
using std::vector;
vector<Vertex> getImageExtent(std::string){
    vector<Vertex> extent;
    return extent;
}
void BundleParser::getBundlePath(){
    size_t pos = mtlFilePath.find_last_of("/\\");
    if (pos != std::string::npos) {
        bundlePath = mtlFilePath.substr(0, pos);
        fileIdentifier = mtlFilePath.substr(pos + 18,pos+27);
    }
    else{
        std::cerr<<"not a correct absolute path"<<std::endl;
    }
}
BundleParser::BundleParser(std::string filePath) : mtlFilePath(filePath){
    getBundlePath();
}
void BundleParser::PrintInfo(){
    std::cout<<"bundle path:"<<bundlePath<<std::endl;
    std::cout<<"fileIdentifier:"<<fileIdentifier<<std::endl;
    for (std::unordered_map<int, std::string>::iterator rasterInfo = TIFFpathParser.begin(); rasterInfo != TIFFpathParser.end(); rasterInfo++)
        std::cout<<"band"<<rasterInfo->first<<" is in "<<rasterInfo->second<<std::endl;
    for (std::unordered_map<std::string, std::string>::iterator para = projectionParams.begin(); para != projectionParams.end(); para++)
        std::cout<<para->first<<" : "<<para->second<<std::endl;
}
void Landsat8BundleParser::readTIffPath(){
    std::ifstream mtlFile(mtlFilePath);
    if (!mtlFile.is_open()) {
        std::cerr << "Error: Could not open MTL file." << std::endl;
        return;
    }
    std::string line;
    while (std::getline(mtlFile, line)) {
        if (line.find("FILE_NAME_BAND_") != std::string::npos) {
            std::istringstream iss(line);
            std::string key, value;
            iss >> key >> value >> value;
            value = value.substr(1,value.size()-2);
            if (key.substr(15).size() > 1)
                continue;
            int bandNumber = std::stoi(key.substr(15));
            TIFFpathParser[bandNumber] = value;
        }
    }
    mtlFile.close();
}
void Landsat8BundleParser::readProjection(){
    std::ifstream mtlFile(mtlFilePath);
    std::string line;
    if (!mtlFile.is_open()) {
        std::cerr << "Error: Could not open MTL file." << std::endl;
        return;
    }
    bool inGroup = false;
    while (std::getline(mtlFile, line)) {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (line == "GROUP = LEVEL1_PROJECTION_PARAMETERS") {
                inGroup = true;
                continue;
            }
            if (line == "END_GROUP = LEVEL1_PROJECTION_PARAMETERS") {
                inGroup = false;
                continue;
            }
            if (inGroup) {
                std::string key, value;
                size_t equalPos = line.find('=');
                if (equalPos != std::string::npos) {
                    key = line.substr(0, equalPos);
                    value = line.substr(equalPos + 1);
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t\""));
                    value.erase(value.find_last_not_of(" \t\"") + 1);
                    projectionParams[key] = value; 
                }
            }
        }
        mtlFile.close();
}
