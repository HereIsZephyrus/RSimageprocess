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
struct Range{
    glm::vec2 upleft,upright,downleft,downright;
};
class BundleParser{
protected:
    std::string mtlFilePath,bundlePath,fileIdentifier;
    void splitBundlePath();
    virtual void readTIffPath() = 0;
    virtual void readProjection() = 0;
    virtual void readLocation() = 0;
public:
    explicit BundleParser(std::string filePath);
    void PrintInfo();
    virtual std::string getWaveLength(int bandindex) = 0;
    std::string getFileIdentifer() const {return fileIdentifier;}
    std::string getBundlePath() const {return bundlePath;}
    std::unordered_map<int, std::string> TIFFpathParser;
    std::unordered_map<std::string, std::string> projectionParams;
    Range geographic,projection;
};
class Landsat8BundleParser : public BundleParser{
    void readTIffPath() override;
    void readProjection() override;
    void readLocation() override;
    static constexpr std::string wavelength[7] = {
            "0.433-0.450","0.450-0.515","0.525-0.600","0.640-0670",
            "0.850-0.880","1.570-1.800","2.100-2.300"
        };
public:
    explicit Landsat8BundleParser(std::string filePath):BundleParser(filePath){
        readTIffPath();
        readProjection();
        readLocation();
    }
    std::string getWaveLength(int bandindex) override{
        if (bandindex >= 7){
            std::cerr<<"out of range"<<std::endl;
            return "";
        }
        else
            return wavelength[bandindex];
    }
};
typedef std::shared_ptr<BundleParser> pParser;
#endif /* interface_hpp */
