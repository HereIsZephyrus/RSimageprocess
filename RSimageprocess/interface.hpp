//
//  interface.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/11/24.
//

#ifndef interface_hpp
#define interface_hpp
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <ogrsf_frmts.h>
#include <ogr_api.h>
#include <ogr_geometry.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <ogr_spatialref.h>
#include "OpenGL/graphing.hpp"
using std::vector;
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
    void ShowInfo();
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
class ROIparser{
    OGRSpatialReference geographicSRS, projectionSPS;
    OGRCoordinateTransformation *transformation;
    std::vector<ClassType> elements;
    glm::vec3 splitColor(std::string colorStr);
public:
    explicit ROIparser(std::string filePath);
    ~ROIparser(){
        OGRCoordinateTransformation::DestroyCT(transformation);
    }
    const std::vector<ClassType>& getCollection(){return elements;}
};
#endif /* interface_hpp */
