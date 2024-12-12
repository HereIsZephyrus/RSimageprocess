//
//  interface.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/11/24.
//

#ifndef interface_hpp
#define interface_hpp
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include "OpenGL/graphing.hpp"
using std::vector;
vector<Vertex> getImageExtent(std::string);
class ImageReader{
    
};
class BundleParser{
protected:
    std::string mtlFilePath,bundlePath,fileIdentifier;
    std::unordered_map<int, std::string> TIFFpathParser;
    std::unordered_map<std::string, std::string> projectionParams;
    void getBundlePath();
    virtual void readTIffPath() = 0;
    virtual void readProjection() = 0;
public:
    explicit BundleParser(std::string filePath);
    void PrintInfo();
};
class Landsat8BundleParser : public BundleParser{
    void readTIffPath() override;
    void readProjection() override;
public:
    explicit Landsat8BundleParser(std::string filePath):BundleParser(filePath){
        readTIffPath();
        readProjection();
    };
};
typedef std::shared_ptr<BundleParser> pParser;
#endif /* interface_hpp */
