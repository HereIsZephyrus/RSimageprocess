//
//  commander.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/10/24.
//

#include "commander.hpp"
#include "window.hpp"
#include "../interface.hpp"
#include "../classification/classifybase.hpp"
#include "../classification/unsupervise_classifier.hpp"
#include "../classification/supervise_classifier.hpp"

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
void Layer::draw(){
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
        static bool useGlobalRange = true;
        if (ImGui::BeginCombo("选择一种方式", strechList[selectedItem].second.c_str())) {
            for (int i = 0; i < strechList.size(); i++) {
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
void Layer::filterBands(){
    using methodStrMap = std::unordered_map<BandProcessType,std::string>;
    static const methodStrMap methodList{
        {BandProcessType::meanBlur,"均值滤波"},
        {BandProcessType::gaussianBlur,"高斯滤波"},
        {BandProcessType::laplacian,"Laplacian变换"},
        {BandProcessType::sobel,"Sobel变换"},
    };
    static bool toSetParas = false;
    static BandProcessType selectedAddItem = BandProcessType::meanBlur;
    ImGui::OpenPopup("Choose Filter Methods");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(pos);
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    if (ImGui::BeginPopup("Choose Filter Methods")) {
        ImGui::PushFont(gui::chineseFont);
        ImGui::BeginChild("##selectable table", ImVec2(150, 200), true);
        ImGui::Text("<可选操作>");
        for (methodStrMap::const_iterator method = methodList.begin(); method != methodList.end(); method++){
            bool isSelected = (selectedAddItem == method->first);
            if (ImGui::Selectable(method->second.c_str(),isSelected)) {
                selectedAddItem = method->first;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##adding table", ImVec2(250, 200), true);
        ImGui::Text("<待执行操作>");
        for (std::vector<BandProcess>::iterator process = buffer.processes.begin(); process != buffer.processes.end(); process++){
            ImGui::Text("%s",(methodList.at(process->getType()) + ": " + process->printParas()).c_str());
        }
        ImGui::EndChild();
        BufferRecorder& buffer = BufferRecorder::getBuffer();
        if (ImGui::Button("确认##texture")) {
            raster->deleteTexture();
            raster->generateTexture(buffer.processes);
            gui::toShowSpaceFilter = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("取消##texture")) {
            gui::toShowSpaceFilter = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("清空##texture")) {
            buffer.processes.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("添加") || toSetParas){
            toSetParas = true;
            static char inputBuffer[10] = "";
            std::map<std::string,float> para;
            ImGui::Text("带宽:");
            ImGui::SameLine();
            ImGui::PushItemWidth(40);
            ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer),ImGuiInputTextFlags_CharsDecimal);
            ImGui::PopItemWidth();
            if (ImGui::Button("确认")) {
                para["bandwidth"] = std::stoi(inputBuffer);
                buffer.processes.push_back(BandProcess(selectedAddItem,para));
                inputBuffer[0] = '\0';
                toSetParas = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("取消##para")) {
                inputBuffer[0] = '\0';
                toSetParas = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::PopFont();
        ImGui::EndPopup();
    }
}
void Layer::unsupervised(){
    using methodStrMap = std::unordered_map<ClassifierType,std::string>;
    static const methodStrMap methodList{
        {ClassifierType::isodata,"ISODATA"},
        {ClassifierType::kmean,"K-mean"},
    };
    ImGui::OpenPopup("Unsupervised");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(pos);
    static ClassifierType selectedItem = ClassifierType::isodata;
    if (ImGui::BeginPopup("Unsupervised")){
        ImGui::PushFont(gui::chineseFont);
        if (ImGui::BeginCombo("选择一种方式", methodList.at(selectedItem).c_str())) {
            for (methodStrMap::const_iterator method = methodList.begin(); method != methodList.end(); method++){
                bool isSelected = (selectedItem == method->first);
                if (ImGui::Selectable(method->second.c_str(), isSelected))
                    selectedItem = method->first;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        static char inputBuffer[10] = "";
        ImGui::Text("分类数量:");
        ImGui::SameLine();
        ImGui::PushItemWidth(40);
        ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer),ImGuiInputTextFlags_CharsDecimal);
        ImGui::PopItemWidth();
        ClassMapper& classMapper = ClassMapper::getClassMap();
        int currentNum = std::stoi(inputBuffer);
        if (currentNum > 1)
            classMapper.setTotalNum(currentNum);
        if (ImGui::Button("确认")) {
            inputBuffer[0] = '\0';
            ClassifyImage(selectedItem);
            gui::toShowUnsupervised = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("取消")) {
            inputBuffer[0] = '\0';
            gui::toShowUnsupervised = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopFont();
        ImGui::EndPopup();
    }
}
void Layer::supervised(){
    using methodStrMap = std::unordered_map<ClassifierType,std::string>;
    static const methodStrMap methodList{
        {ClassifierType::naiveBayes,"朴素Bayes"},
        {ClassifierType::fisher,"Fisher"},
        {ClassifierType::svm,"SVM"},
        {ClassifierType::bp,"BP"},
        {ClassifierType::rf,"RF"},
    };
    ImGui::OpenPopup("Supervised");
    ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(pos);
    static ClassifierType selectedItem = ClassifierType::rf;
    if (ImGui::BeginPopup("Supervised")){
        ImGui::PushFont(gui::chineseFont);
        if (ImGui::BeginCombo("选择一种方式", methodList.at(selectedItem).c_str())) {
            for (methodStrMap::const_iterator method = methodList.begin(); method != methodList.end(); method++){
                bool isSelected = (selectedItem == method->first);
                if (ImGui::Selectable(method->second.c_str(), isSelected))
                    selectedItem = method->first;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        static char inputBuffer[10] = "";
        ImGui::Text("分类数量:");
        ImGui::SameLine();
        ImGui::PushItemWidth(40);
        ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer),ImGuiInputTextFlags_CharsDecimal);
        ImGui::PopItemWidth();
        if (ImGui::Button("确认")) {
            inputBuffer[0] = '\0';
            ClassifyImage(selectedItem);
            gui::toShowUnsupervised = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("取消")) {
            inputBuffer[0] = '\0';
            gui::toShowUnsupervised = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopFont();
        ImGui::EndPopup();
    }
}
void Layer::ClassifyImage(ClassifierType classifierType){
    unsigned char* classified = nullptr;
    ClassMapper& classMapper = ClassMapper::getClassMap();
    if (classifierType == ClassifierType::isodata){
        ISODATA solver(classMapper.getTotalNum());
        solver.Classify(raster->getBands(), raster->getToAverage(), classified);
    }
    else if (classifierType == ClassifierType::kmean){;
    }else{
        std::shared_ptr<Classifier> classifier = nullptr;
        switch (classifierType) {
            case ClassifierType::naiveBayes:
                classifier = std::make_shared<NaiveBayesClassifier>();
                break;
            case ClassifierType::fisher:
                classifier = std::make_shared<FisherClassifier>();
                break;
            case ClassifierType::svm:
                classifier = std::make_shared<SVMClassifier>();
                break;
            case ClassifierType::bp:
                classifier = std::make_shared<BPClassifier>();
                break;
            case ClassifierType::rf:
                classifier = std::make_shared<RandomForestClassifier>();
                break;
            default:
                break;
        }
        if (classifier == nullptr)
            return;
        //classifier->Train(<#const Dataset &dataset#>);
        //classifier->Classify(<#const std::vector<Band> &bands#>, classified);
        //classifier->Examine(<#const Dataset &samples#>);
    }
    raster->deleteTexture();
    raster->generateClassifiedTexture(classified);
    delete [] classified;
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
    gui::toShowManageBand = true;
    image->manageBands();
    addLayer(newLayer);
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    buffer.selectedLayer = newLayer;
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
void LayerManager::draw(){
    pLayer current = tail;
    while (current != nullptr){
        if (current->getVisble())
            current->draw();
        current = current->prev;
    }
}
