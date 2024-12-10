//
//  window.cpp
//  data_structure
//
//  Created by ChanningTong on 10/21/24.
//

#include <iostream>
#include <cmath>
#include <cstring>
#include <string>
#include <algorithm>
#include <glm/glm.hpp>
#include "window.hpp"
#include "graphing.hpp"

void WindowParas::InitParas(){
    glfwGetWindowContentScale(window, &xScale, &yScale);
    SCREEN_LEFT = SIDEBAR_WIDTH * xScale;   SCREEN_BOTTON = 0;
    SCREEN_WIDTH = WINDOW_WIDTH * xScale - SCREEN_LEFT;   SCREEN_HEIGHT = WINDOW_HEIGHT * yScale - SCREEN_BOTTON;
}
GLfloat WindowParas::screen2normalX(GLdouble screenX){
    return  (2.0f * static_cast<GLfloat>((screenX - SCREEN_LEFT)/ SCREEN_WIDTH * xScale)) - 1.0f;
}
GLfloat WindowParas::screen2normalY(GLdouble screenY){
    return 1.0f - (2.0f * static_cast<GLfloat>((screenY - SCREEN_BOTTON) / SCREEN_HEIGHT * yScale));
}
GLfloat WindowParas::normal2orthoX(GLfloat normalX){
    GLfloat left = -SCREEN_WIDTH / xScale / 2.0f;
    GLfloat right = SCREEN_WIDTH / xScale / 2.0f;
    return  left + (right - left) * (normalX + 1) / 2;
}
GLfloat WindowParas::normal2orthoY(GLfloat normalY){
    GLfloat button = -SCREEN_HEIGHT / yScale / 2.0f;
    GLfloat top = SCREEN_HEIGHT / yScale / 2.0f;
    return  button + (top - button) * (normalY + 1) / 2;
}
void windowPosChangeCallback(GLFWwindow* window, int xpos, int ypos){
    WindowParas& windowPara = WindowParas::getInstance();
    glViewport(windowPara.SCREEN_LEFT, windowPara.SCREEN_BOTTON, windowPara.SCREEN_WIDTH, windowPara.SCREEN_HEIGHT);
}
void windowRefreshCallback(GLFWwindow* window){
    WindowParas& windowPara = WindowParas::getInstance();
    glViewport(windowPara.SCREEN_LEFT, windowPara.SCREEN_BOTTON, windowPara.SCREEN_WIDTH, windowPara.SCREEN_HEIGHT);
}
int initOpenGL(GLFWwindow *&window,std::string windowName) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    WindowParas& windowPara = WindowParas::getInstance();
    window = glfwCreateWindow(windowPara.WINDOW_WIDTH, windowPara.WINDOW_HEIGHT, windowName.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()){
        std::cerr << "Failed to initialize GLEW"<<std::endl;
        return -2;
    }
    windowPara.InitParas();
    glViewport(windowPara.SCREEN_LEFT, windowPara.SCREEN_BOTTON, windowPara.SCREEN_WIDTH, windowPara.SCREEN_HEIGHT);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    const GLubyte* version = glGetString(GL_VERSION);
    glfwSetWindowPosCallback(window, windowPosChangeCallback);
    glfwSetWindowRefreshCallback(window, windowRefreshCallback);
    std::cout<<version<<std::endl;
    HAS_INIT_OPENGL_CONTEXT = true;
    return 0;
}
namespace gui {
ImFont *englishFont = nullptr,*chineseFont = nullptr;
int Initialization(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    englishFont = io.Fonts->AddFontFromFileTTF("/Users/channingtong/Program/RSimageprocess/ImGUIopengl3/Arial.ttf", 24.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
    chineseFont = io.Fonts->AddFontFromFileTTF("/Users/channingtong/Program/RSimageprocess/ImGUIopengl3/Songti.ttc", 24.0f,nullptr,io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    io.Fonts->Build();
    return  0;
}

void DrawBasic() {
    WindowParas& windowPara = WindowParas::getInstance();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(windowPara.SIDEBAR_WIDTH, windowPara.WINDOW_HEIGHT));
    
    ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::End();
    return;
}
}
