//
//  window.hpp
//  data_structure
//
//  Created by ChanningTong on 10/21/24.
//

#ifndef window_hpp
#define window_hpp

#include <cstring>
#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "graphing.hpp"

inline bool HAS_INIT_OPENGL_CONTEXT = false;
int initOpenGL(GLFWwindow *&window,std::string windowName);
class WindowParas{
public:
    static WindowParas& getInstance(){
        static WindowParas instance;
        return instance;
    }
    WindowParas(const WindowParas&) = delete;
    void operator=(const WindowParas&) = delete;
    GLFWwindow * window;
    const GLint WINDOW_WIDTH = 1150;
    const GLint WINDOW_HEIGHT = 900;
    const GLint SIDEBAR_WIDTH = 250;
    GLint SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_LEFT,SCREEN_BOTTON;
    GLfloat xScale,yScale;
    GLfloat screen2normalX(GLdouble screenX);
    GLfloat screen2normalY(GLdouble screenY);
    void InitParas();
private:
    WindowParas() {}
};

void windowPosChangeCallback(GLFWwindow* window, int xpos, int ypos);
void windowRefreshCallback(GLFWwindow* window);
namespace gui {
constexpr GLfloat detactBias = 0.48f;
constexpr GLfloat outboundBias = 2.0f;
constexpr GLfloat borderDetectRange = 20.0f;
constexpr GLfloat dragCameraSpeed = 6.0f;
constexpr GLfloat doubleClickBias = 0.2f;
extern ImFont *englishFont,*chineseFont;
extern bool toImportImage,toImportROI;
extern bool toShowStatistic,toShowManageBand,toShowStrechLevel;
int Initialization(GLFWwindow* window);
void DrawBasic();
bool DrawPopup();
void RenderLayerTree();
void RenderWorkspace();
void ImportImage();
void ShowStatistic();
void ManageBands();
void ChooseStrechLevel();
}
#endif /* window_hpp */
