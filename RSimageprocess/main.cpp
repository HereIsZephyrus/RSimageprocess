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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "gdal.h"

int main(int argc, const char * argv[]) {
    GLFWwindow *& window = WindowParas::getInstance().window;
    if (!HAS_INIT_OPENGL_CONTEXT && initOpenGL(window,"2025Autumn数字图像处理") != 0)
        return -1;
    //InitResource(window);
    gui::Initialization(window);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gui::DrawBasic();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    WindowParas::getInstance().window = nullptr;
    return 0;
}
