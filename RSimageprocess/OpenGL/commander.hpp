//
//  commander.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/10/24.
//

#ifndef commander_hpp
#define commander_hpp
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <variant>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "graphing.hpp"
#include "camera.hpp"
#include "../interface.hpp"
#include "../classification/classifybase.hpp"

enum class ClassifierType{
    fisher,
    svm,
    bp,
    rf,
    isodata,
    kmean,
};
class LayerManager;
class Layer{
    std::unique_ptr<Image> raster;
    std::unique_ptr<ROIcollection> vector;
    std::shared_ptr<BundleParser> parserRaster;
    std::shared_ptr<ROIparser> parserVector;
    std::string name;
    bool visble;
    std::vector<Sample> dataset;
    std::string getFileName(std::string resourcePath);
    std::string getIndicator(int index){return raster->getIndicator(index);}
    void TrainROI();
public:
    friend LayerManager;
    Layer(std::string layerName,std::string resourcePath):
    name(layerName),prev(nullptr),next(nullptr),visble(true),raster(nullptr),parserRaster(nullptr),parserVector(nullptr){
        //vector = std::make_unique<ROIcollection>(resourcePath);
    }
    Layer(std::string layerName, const std::vector<Vertex>& vertices):
    name(layerName),prev(nullptr),next(nullptr),visble(true),vector(nullptr),parserRaster(nullptr),parserVector(nullptr){
        raster = std::make_unique<Image>(vertices);
        //vector = std::make_unique<ROIcollection>();
    }
    void draw();
    bool BuildLayerStack();
    std::string getName() const{return name;}
    std::shared_ptr<Layer> prev,next;
    Extent getExtent() const{return raster->getExtent();}
    bool getVisble() const {return visble;}
    void toggleVisble() {visble = !visble;}
    void showStatistic() const;
    void exportImage() const{
        std::string filePath = parserRaster->getBundlePath() + '/' + parserRaster->getFileIdentifer() + raster->getTextureStatus() + ".png";
        raster->exportImage(filePath);
    }
    void importROI(std::shared_ptr<ROIparser> parser);
    void manageBands() const{raster->manageBands();}
    void averageBands() {raster->averageBands();}
    void strechBands();
    void filterBands();
    void unsupervised();
    void supervised();
    void resetBandIndex(){raster->resetIndex();}
    void ClassifyImage(ClassifierType classifierType);
};
class LayerManager{
    using pLayer = std::shared_ptr<Layer>;
    pLayer head;
    pLayer tail;
    LayerManager() : head(nullptr), tail(nullptr) {}
    LayerManager(const LayerManager&) = delete;
    LayerManager& operator=(const LayerManager&) = delete;
public:
    static LayerManager& getLayers() {
        static LayerManager instance;
        return instance;
    }
    ~LayerManager() {
        while (head != nullptr) {
            pLayer temp = head;
            head = head->next;
            temp = nullptr;
        }
    }
    void addLayer(pLayer newLayer);
    void importlayer(std::shared_ptr<BundleParser> parser);
    void removeLayer(pLayer deleteLayer);
    void moveLayerUp(pLayer swapLayer);
    void moveLayerDown(pLayer swapLayer);
    void printLayerTree();
    void draw();
};
class BufferRecorder{
public:
    static BufferRecorder& getBuffer(){
        static BufferRecorder instance;
        return instance;
    }
    BufferRecorder(const BufferRecorder&) = delete;
    void operator = (const BufferRecorder&) = delete;
    GLboolean keyRecord[GLFW_KEY_LAST+1],pressLeft,pressRight,pressCtrl,pressShift,pressAlt,doubleCliked;
    std::shared_ptr<Layer> selectedLayer;
    std::vector<BandProcess> processes;
    void initIO(GLFWwindow* window);
private:
    BufferRecorder(){}
};
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
#endif /* commander_hpp */
