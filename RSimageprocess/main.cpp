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
    //setTestDataset();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gui::DrawBasic();
        layerManager.draw();
        if (!gui::DrawPopup())
            camera.processKeyboard(window);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
void Initialization(GLFWwindow *& window){
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
        {glm::vec3{40,115,0.0},{1.0,1.0,1.0}},
        {glm::vec3{40,120,0.0},{1.0,1.0,1.0}},
        {glm::vec3{50,120,0.0},{1.0,1.0,1.0}},
        {glm::vec3{50,115,0.0},{1.0,1.0,1.0}}
    };
    std::vector<Vertex> test_points_alter{
        {glm::vec3{42,118,0.0},{0.0,1.0,1.0}},
        {glm::vec3{42,122,0.0},{0.0,1.0,1.0}},
        {glm::vec3{52,122,0.0},{0.0,1.0,1.0}},
        {glm::vec3{52,118,0.0},{0.0,1.0,1.0}}
    };
    ROI test(test_points);
    LayerManager& layerManager = LayerManager::getLayers();
    Camera2D& camera = Camera2D::getView();
    std::shared_ptr<Layer> testLayer1 = std::make_shared<Layer>("testLayer1",test_points);
    std::shared_ptr<Layer> testLayer2 = std::make_shared<Layer>("testLayer2",test_points_alter);
    layerManager.addLayer(testLayer1);
    layerManager.addLayer(testLayer2);
    camera.setExtent(test.getExtent());
}
