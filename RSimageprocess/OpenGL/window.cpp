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
#include "commander.hpp"

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
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "Maximum texture size: " << maxTextureSize << std::endl;
    return 0;
}
namespace gui {
ImFont *englishFont = nullptr,*chineseFont = nullptr;
bool toImportImage = false,toImportROI = false;
bool toShowStatistic = false,toShowManageBand = false,toShowStrechLevel = false,toShowSpaceFilter = false,toShowUnsupervised = false,toShowSupervised = false,toShowPrecision = false;
int Initialization(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    englishFont = io.Fonts->AddFontFromFileTTF("/Users/channingtong/Program/RSimageprocess/ImGUIopengl3/Arial.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
    chineseFont = io.Fonts->AddFontFromFileTTF("/Users/channingtong/Program/RSimageprocess/ImGUIopengl3/Songti.ttc", 20.0f,nullptr,io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    io.Fonts->Build();
    return 0;
}

void DrawBasic() {
    WindowParas& windowPara = WindowParas::getInstance();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(windowPara.SIDEBAR_WIDTH, windowPara.WINDOW_HEIGHT));
    ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    RenderLayerTree();
    RenderWorkspace();
    ImGui::End();
    return;
}
bool DrawPopup(){
    if (toImportImage){
        ImportImage();
        return true;
    }
    if (toImportROI){
        ImportROI();
        return true;
    }
    if (toShowStatistic){
        ShowStatistic();
        return true;
    }
    if (toShowManageBand){
        ManageBands();
        return true;
    }
    if (toShowStrechLevel){
        ChooseStrechLevel();
        return true;
    }
    if (toShowSpaceFilter){
        FilterBands();
        return true;
    }
    if (toShowUnsupervised){
        UnsupervisedClassify();
        return  true;
    }
    if (toShowSupervised){
        SupervisedClassify();
        return true;
    }
    if (toShowPrecision){
        showPrecision();
        return true;
    }
    return false;
}
void RenderLayerTree(){
    WindowParas& windowPara = WindowParas::getInstance();
    ImGui::BeginChild("Layers",ImVec2(0,windowPara.WINDOW_HEIGHT / 3));
    LayerManager& layerManager = LayerManager::getLayers();
    layerManager.printLayerTree();
    ImGui::EndChild();
}
void RenderWorkspace(){
    WindowParas& windowPara = WindowParas::getInstance();
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    ImGui::BeginChild("Workspace",ImVec2(0,windowPara.WINDOW_HEIGHT * 3 / 5));
    ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 ButtonSize = ImVec2(windowPara.SIDEBAR_WIDTH * 3 / 7, 50);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(12.0f, 8.0f);
    ImGui::PushFont(gui::chineseFont);
    if (ImGui::Button("导入遥感影像",ButtonSize))
        toImportImage = true;
    ImGui::SameLine();
    if (ImGui::Button("导入影像序列",ButtonSize))
        toImportImage = true;
    if (buffer.selectedLayer != nullptr){
        std::string visbleButtonStr = "隐藏图层";
        if (!buffer.selectedLayer->getLayerVisble())
            visbleButtonStr = "显示图层";
        if (ImGui::Button(visbleButtonStr.c_str(),ButtonSize))
            buffer.selectedLayer->toggleLayerVisble();
        ImGui::SameLine();
        if (ImGui::Button("导出影像",ButtonSize))
            buffer.selectedLayer->exportImage();
        if (buffer.selectedLayer->hasROI()){
            if (buffer.selectedLayer->getROIVisble()){
                if (ImGui::Button("隐藏ROI",ButtonSize))
                    buffer.selectedLayer->toggleROIVisble();
            }else{
                if (ImGui::Button("显示ROI",ButtonSize))
                    buffer.selectedLayer->toggleROIVisble();
            }
        }else{
            if (ImGui::Button("导入ROI",ButtonSize))
                toImportROI = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("计算变化",ButtonSize))
            toImportROI = true;
        if (ImGui::Button("查看信息",ButtonSize))
            toShowStatistic = true;
        ImGui::SameLine();
        if (ImGui::Button("波段重组",ButtonSize)){
            toShowManageBand = true;
            buffer.selectedLayer->resetBandIndex();
        }
        if (ImGui::Button("直方图均衡化",ButtonSize))
            buffer.selectedLayer->averageBands();
        ImGui::SameLine();
        if (ImGui::Button("对比度拉伸",ButtonSize))
            toShowStrechLevel = true;
        if (ImGui::Button("谱域滤波",ButtonSize)){
            
        }
        ImGui::SameLine();
        if (ImGui::Button("空间滤波",ButtonSize)){
            toShowSpaceFilter = true;
        }
        if (ImGui::Button("无监督分类",ButtonSize)){
            toShowUnsupervised = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("监督分类",ButtonSize)){
            toShowSupervised = true;
        }
        visbleButtonStr = "隐藏特征";
        if (!buffer.selectedLayer->getFeatureVisble())
            visbleButtonStr = "显示特征";
        if (buffer.selectedLayer->hasClassified())
            if (ImGui::Button(visbleButtonStr.c_str(),ButtonSize))
                buffer.selectedLayer->toggleFeatureVisble();
        ImGui::SameLine();
        if (buffer.selectedLayer->hasClassified())
            if (ImGui::Button("显示精度",ButtonSize))
                toShowPrecision = true;
    }
    style.FramePadding = ImVec2(4.0f, 2.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    ImGui::PopFont();
    ImGui::EndChild();
}
void ImportImage(){
    static char inputBuffer[256] = "";
    ImGui::PushFont(gui::chineseFont);
    ImGui::OpenPopup("Import Image");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(pos);
    if (ImGui::BeginPopup("Import Image")) {
        ImGui::Text("输入遥感集MTL文件");
        ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer));
        if (ImGui::Button("确认")) {
            pParser parser = std::make_shared<Landsat8BundleParser>(inputBuffer);
            LayerManager& layerManager = LayerManager::getLayers();
            layerManager.importlayer(parser);
            inputBuffer[0] = '\0';
            toImportImage = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("取消")) {
            inputBuffer[0] = '\0';
            toImportImage = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopFont();
}
void ImportROI(){
    static char inputBuffer[256] = "";
    ImGui::PushFont(gui::chineseFont);
    ImGui::OpenPopup("Import ROI");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(pos);
    if (ImGui::BeginPopup("Import ROI")) {
        ImGui::Text("输入ROI的GeoJson文件");
        ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer));
        if (ImGui::Button("确认")) {
            std::shared_ptr<ROIparser> parser = std::make_shared<ROIparser>(inputBuffer);
            BufferRecorder& buffer = BufferRecorder::getBuffer();
            if (buffer.selectedLayer != nullptr){
                buffer.selectedLayer->importROI(parser);
            }
            inputBuffer[0] = '\0';
            toImportROI = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("取消")) {
            inputBuffer[0] = '\0';
            toImportROI = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopFont();
}
void ShowStatistic(){
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer->showStatistic();
}
void ManageBands(){
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer->manageBands();
}
void ChooseStrechLevel(){
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer->strechBands();
}
void FilterBands(){
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer->filterBands();
}
void UnsupervisedClassify(){
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer->unsupervised();
}
void SupervisedClassify(){
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer->supervised();
}
void showPrecision(){
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer->showPrecision();
}
}
