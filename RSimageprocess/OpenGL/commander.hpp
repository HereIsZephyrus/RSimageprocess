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
class BufferRecorder{
public:
    static BufferRecorder& getBuffer(){
        static BufferRecorder instance;
        return instance;
    }
    BufferRecorder(const BufferRecorder&) = delete;
    void operator = (const BufferRecorder&) = delete;
    GLboolean keyRecord[GLFW_KEY_LAST+1],pressLeft,pressRight,pressCtrl,pressShift,pressAlt,doubleCliked;
    void initIO(GLFWwindow* window);
private:
    BufferRecorder(){}
};
enum class LayerType{
    raster,
    vector
};
class Layer{
    std::variant<std::unique_ptr<Image>,std::unique_ptr<ROIcollection>> object;
    std::string name;
    LayerType type;
    std::string getFileName(std::string resourcePath);
public:
    Layer(std::string resourcePath, LayerType layerType);
    Layer(std::string layerName, const std::vector<Vertex>& vertices):
    name(layerName),prev(nullptr),next(nullptr)//deprecated, just for test
    {
        object = std::make_unique<Image>(vertices);
    }
    void Draw();
    void BuildLayerStack();
    std::string getName() const{return name;}
    std::shared_ptr<Layer> prev,next;
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
    void addLayer(std::string resourcePath, LayerType layerType);
    void importlayer(const Landsat8BundleParser& parser){
        
    }
    void removeLayer(pLayer deleteLayer);
    void moveLayerUp(pLayer swapLayer);
    void moveLayerDown(pLayer swapLayer);
    void printLayerTree();
    void Draw();
};
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
#endif /* commander_hpp */
