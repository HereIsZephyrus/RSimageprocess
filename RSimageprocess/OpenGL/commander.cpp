//
//  commander.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/10/24.
//

#include "commander.hpp"
#include "window.hpp"
#include "../interface.hpp"

void BufferRecorder::initIO(GLFWwindow* window){
    memset(keyRecord, GL_FALSE, sizeof(keyRecord));
    pressLeft = GL_FALSE;
    pressRight = GL_FALSE;
    pressAlt = GL_FALSE;
    pressShift = GL_FALSE;
    pressCtrl = GL_FALSE;
    doubleCliked = GL_FALSE;
    selectedLayer = nullptr;
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
}
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    Camera2D::getView().processScroll(window, xoffset, yoffset, buffer.pressCtrl, buffer.pressAlt);
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    if (action == GLFW_PRESS){
        buffer.keyRecord[key] = GL_TRUE;
        if (buffer.keyRecord[GLFW_KEY_LEFT_CONTROL] || buffer.keyRecord[GLFW_KEY_RIGHT_CONTROL]) buffer.pressCtrl = GL_TRUE;
        if (buffer.keyRecord[GLFW_KEY_LEFT_SHIFT] || buffer.keyRecord[GLFW_KEY_RIGHT_SHIFT])  buffer.pressShift = GL_TRUE;
        if (buffer.keyRecord[GLFW_KEY_LEFT_ALT] || buffer.keyRecord[GLFW_KEY_RIGHT_ALT])    buffer.pressAlt = GL_TRUE;
    }
    if (action == GLFW_RELEASE){
        //std::cout<<"release "<<key<<std::endl;
        buffer.keyRecord[key] = GL_FALSE;
        if (!buffer.keyRecord[GLFW_KEY_LEFT_CONTROL] && !buffer.keyRecord[GLFW_KEY_RIGHT_CONTROL]) buffer.pressCtrl = GL_FALSE;
        if (!buffer.keyRecord[GLFW_KEY_LEFT_SHIFT] && !buffer.keyRecord[GLFW_KEY_RIGHT_SHIFT])  buffer.pressShift = GL_FALSE;
        if (!buffer.keyRecord[GLFW_KEY_LEFT_ALT] && !buffer.keyRecord[GLFW_KEY_RIGHT_ALT])    buffer.pressAlt = GL_FALSE;
    }
}
std::string Layer::getFileName(std::string resourcePath){
    return resourcePath;
}
void Layer::Draw(){
    using pImage = std::unique_ptr<Image>;
    using pROI = std::unique_ptr<ROIcollection>;
    if (raster != nullptr)  raster->draw();
    if (vector != nullptr)  vector->draw();
}
bool Layer::BuildLayerStack(){
    const ImGuiTreeNodeFlags propertyFlag = ImGuiTreeNodeFlags_Leaf;
    bool clicked = false;
    if (vector != nullptr){
        if (ImGui::TreeNodeEx("vector", propertyFlag)){
            ImGui::TreePop();
            if (ImGui::IsItemClicked())
                clicked = true;
        }
    }
    const std::vector<Band>& bands = raster->getBands();
    int counter = 0;
    for (std::vector<Band>::const_reverse_iterator band = bands.rbegin(); band != bands.rend(); band++){
        std::ostringstream nameOS;
        std::string bandIndicator = getIndicator(counter);
        nameOS<<"band"<<++counter<<std::setprecision(1)<<":"<<band->wavelength<<"mm"<<bandIndicator;
        if (ImGui::TreeNodeEx(nameOS.str().c_str(), propertyFlag)){
            ImGui::TreePop();
            if (ImGui::IsItemClicked())
                clicked = true;
        }
    }
    return clicked;
}
void Layer::showStatistic() const{
    ImGui::OpenPopup("Statistic Information");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    pos.x /= 2; pos.y /=2;
    ImGui::SetNextWindowPos(pos);
    if (ImGui::BeginPopup("Statistic Information")) {
        ImGui::PushFont(gui::chineseFont);
        ImGuiStyle& style = ImGui::GetStyle();
        style.ItemSpacing = ImVec2(16.0f, 8.0f);
        ImGui::Text("<影像元信息>");
        parser->ShowInfo();
        ImGui::Text("<波段相关系数>");
        raster->showBandCoefficient();
        ImGui::Text("<波段信息>");
        static int showBandIndex = 0;
        ImGui::SameLine();
        ImGui::Text("%s",std::string("band" + std::to_string(showBandIndex + 1)).c_str());
        ImGui::SameLine();
        if (ImGui::ArrowButton("##decrease band index", ImGuiDir_Left))
            showBandIndex = std::max(showBandIndex - 1, 0);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##increase band index", ImGuiDir_Right))
            showBandIndex = std::min(showBandIndex + 1, static_cast<int>(raster->getBands().size() - 1));
        raster->showBandInfo(showBandIndex);
        if (ImGui::Button("确认")) {
            gui::toShowStatistic = false;
            ImGui::CloseCurrentPopup();
        }
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        ImGui::PopFont();
        ImGui::EndPopup();
    }
}
void Layer::strechBands() {
    static constexpr std::array<std::pair<StrechLevel,std::string>,4> strechList{
        std::make_pair(StrechLevel::noStrech,"不拉伸"),
        std::make_pair(StrechLevel::minmaxStrech,"极值线性拉伸"),
        std::make_pair(StrechLevel::percent1Strech,"1%线性拉伸"),
        std::make_pair(StrechLevel::percent2Strech,"2%线性拉伸"),
    };
    ImGui::OpenPopup("Choose Strech Level");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(pos);
    if (ImGui::BeginPopup("Choose Strech Level")) {
        ImGui::PushFont(gui::chineseFont);
        static int selectedItem = 0;
        static bool useGlobalRange = false;
        if (ImGui::BeginCombo("选择一种方式", strechList[selectedItem].second.c_str())) {
            for (int i = 0; i < strechList.size(); ++i) {
                bool isSelected = (selectedItem == i);
                if (ImGui::Selectable(strechList[i].second.c_str(), isSelected))
                    selectedItem = i;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("确认")) {
            raster->strechBands(strechList[selectedItem].first,useGlobalRange);
            gui::toShowStrechLevel = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("取消")) {
            gui::toShowStrechLevel = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("使用全局极值", useGlobalRange))
            useGlobalRange = !useGlobalRange;
        ImGui::PopFont();
        ImGui::EndPopup();
    }
}
void LayerManager::addLayer(pLayer newLayer) {
    if (head == nullptr) {
        head = tail = newLayer;
    } else {
        tail->next = newLayer;
        newLayer->prev = tail;
        tail = newLayer;
    }
}
void LayerManager::importlayer(std::shared_ptr<BundleParser> parser){
    std::vector<Vertex> faceVertices = {
        {glm::vec3(parser->geographic.downleft.x,parser->geographic.downleft.y,0.0),glm::vec3(1.0,1.0,1.0)},
        {glm::vec3(parser->geographic.downright.x,parser->geographic.downright.y,0.0),glm::vec3(1.0,1.0,1.0)},
        {glm::vec3(parser->geographic.upright.x,parser->geographic.upright.y,0.0),glm::vec3(1.0,1.0,1.0)},
        {glm::vec3(parser->geographic.upleft.x,parser->geographic.upleft.y,0.0),glm::vec3(1.0,1.0,1.0)},
    };
    pLayer newLayer = std::make_shared<Layer>(parser->getFileIdentifer(),faceVertices);
    newLayer->parser = parser;
    std::unique_ptr<Image>& image = newLayer->raster;
    for (std::unordered_map<int, std::string>::iterator rasterInfo = parser->TIFFpathParser.begin(); rasterInfo != parser->TIFFpathParser.end(); rasterInfo++){
        std::string imagePath = parser->getBundlePath() + "/" + rasterInfo->second;
        if (rasterInfo->first > 7)
            continue;
        image->LoadNewBand(imagePath,parser->getWaveLength(rasterInfo->first-1));
    }
    image->generateTexture();
    addLayer(newLayer);
    Camera2D& camera = Camera2D::getView();
    camera.setExtent(newLayer->getExtent());
    std::cout<<"imported "<<parser->getFileIdentifer()<<std::endl;
}
void LayerManager::removeLayer(pLayer deleteLayer) {
    if (deleteLayer->prev != nullptr)
        deleteLayer->prev->next = deleteLayer->next;
    else
        head = deleteLayer->next;
    if (deleteLayer->next != nullptr)
        deleteLayer->next->prev = deleteLayer->prev;
    else
        tail = deleteLayer->prev;
    if (BufferRecorder::getBuffer().selectedLayer == deleteLayer)
        BufferRecorder::getBuffer().selectedLayer = nullptr;
    deleteLayer = nullptr;
}
void LayerManager::moveLayerUp(pLayer swapLayer) {
    if (swapLayer == head)
        return;
    pLayer prevLayer = swapLayer->prev;
    if  (prevLayer->prev != nullptr)
        prevLayer->prev->next = swapLayer;
    else
        head = swapLayer;
    swapLayer->prev = prevLayer->prev;
    prevLayer->next = swapLayer->next;
    swapLayer->next = prevLayer;
    prevLayer->prev = swapLayer;
    if (prevLayer->next != nullptr)
        prevLayer->next->prev = prevLayer;
    else
        tail = prevLayer;
}
void LayerManager::moveLayerDown(pLayer swapLayer) {
    if (swapLayer == tail)
        return;
    pLayer nextLayer = swapLayer->next;
    if (nextLayer->next != nullptr)
        nextLayer->next->prev = swapLayer;
    else
        tail = swapLayer;
    swapLayer->next = nextLayer->next;
    nextLayer->prev = swapLayer->prev;
    swapLayer->prev = nextLayer;
    nextLayer->next = swapLayer;
    if (nextLayer->prev != nullptr)
        nextLayer->prev->next = nextLayer;
    else
        head = nextLayer;
}
void LayerManager::printLayerTree(){
    pLayer current = head;
    const ImGuiTreeNodeFlags layerFlag = ImGuiTreeNodeFlags_DefaultOpen;
    while (current != nullptr){
        bool isOpen = ImGui::TreeNodeEx(current->getName().c_str(), layerFlag);
        if (ImGui::IsItemClicked())
            BufferRecorder::getBuffer().selectedLayer = current;
        ImGui::SameLine();
        if (ImGui::ArrowButton(std::string("##UpArrow"+ current->getName()).c_str(), ImGuiDir_Up))
            moveLayerUp(current);
        ImGui::SameLine();
        if (ImGui::ArrowButton(std::string("##DownArrow" + current->getName()).c_str(), ImGuiDir_Down))
            moveLayerDown(current);
        ImGui::SameLine();
        if (ImGui::ArrowButton(std::string("##RightArrow" + current->getName()).c_str(), ImGuiDir_Right))
            Camera2D::getView().setExtent(current->getExtent());
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button(std::string("Del##" + current->getName()).c_str()))
            removeLayer(current);
        ImGui::PopStyleColor(3);
        if (isOpen){
            if (ImGui::IsItemClicked()){}
            if (current->BuildLayerStack())
                BufferRecorder::getBuffer().selectedLayer = current;
            ImGui::TreePop();
        }
        current = current->next;
    }
}
void LayerManager::Draw(){
    pLayer current = tail;
    while (current != nullptr){
        if (current->getVisble())
            current->Draw();
        current = current->prev;
    }
}
