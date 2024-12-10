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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
#endif /* commander_hpp */
