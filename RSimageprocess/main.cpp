//
//  main.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/10/24.
//
#include <iostream>
#include <cstring>
#include <string>
#include <memory>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "OpenGL/graphing.hpp"
#include "OpenGL/window.hpp"
#include "OpenGL/commander.hpp"
#include "OpenGL/camera.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "gdal_priv.h"
#include "gdal.h"
#include "interface.hpp"

static void Initialization(GLFWwindow *&window);
static void setTestDataset();
int main(int argc, const char * argv[]) {
    GLFWwindow *& window = WindowParas::getInstance().window;
    Initialization(window);
    Camera2D& camera = Camera2D::getView();
    LayerManager& layerManager = LayerManager::getLayers();
    setTestDataset();
    pParser parser = std::make_shared<Landsat8BundleParser>("/Users/channingtong/Document/rawTIF/LC08_L2SP_118039_20220315_20220322_02_T1/LC08_L2SP_118039_20220315_20220322_02_T1_MTL.txt");
    parser->PrintInfo();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gui::DrawBasic();
        camera.processKeyboard(window);
        layerManager.Draw();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    //WindowParas::getInstance().window = nullptr;
    return 0;
}
static void Initialization(GLFWwindow *& window){
    if (!HAS_INIT_OPENGL_CONTEXT && initOpenGL(window,"2025Autumn数字图像处理") != 0){
        std::cout<<"init OpenGL failed"<<std::endl;
        return;
    }
    InitResource(window);
    gui::Initialization(window);
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.initIO(window);
    GDALAllRegister();
}
void setTestDataset(){
    std::vector<Vertex> test_points{
        {glm::vec3{115,40,0.0},{1.0,1.0,1.0}},
        {glm::vec3{120,40,0.0},{1.0,1.0,1.0}},
        {glm::vec3{120,50,0.0},{1.0,1.0,1.0}},
        {glm::vec3{115,50,0.0},{1.0,1.0,1.0}}
    };
    std::vector<Vertex> test_points_alter{
        {glm::vec3{118,42,0.0},{0.0,1.0,1.0}},
        {glm::vec3{122,42,0.0},{0.0,1.0,1.0}},
        {glm::vec3{122,52,0.0},{0.0,1.0,1.0}},
        {glm::vec3{118,52,0.0},{0.0,1.0,1.0}}
    };
    //Layer testlayer(te)
    ROI test(test_points);
    LayerManager& layerManager = LayerManager::getLayers();
    Camera2D& camera = Camera2D::getView();
    std::shared_ptr<Layer> testLayer1 = std::make_shared<Layer>("testLayer1",test_points);
    std::shared_ptr<Layer> testLayer2 = std::make_shared<Layer>("testLayer2",test_points_alter);
    layerManager.addLayer(testLayer1);
    layerManager.addLayer(testLayer2);
    camera.setExtent(test.getExtent());
}
