//
//  camera.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/10/24.
//

#ifndef camera_hpp
#define camera_hpp

#include <iostream>
#include <vector>
#include <memory>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Extent{
    GLfloat left,right,top,botton;
    Extent():left(-1),right(1),top(1),botton(-1){}
};
class Camera2D {
public:
    static Camera2D& getView(){
        static Camera2D instance;
        return instance;
    }
    void processKeyboard(GLFWwindow* window);
    void processScroll(GLFWwindow* window, double xoffset, double yoffset, bool pressCtrl, bool pressAlt);
    void zoomInOut(float yOffset);
    glm::mat4 getProjectionMatrix() const {return projectionMatrix;}
    glm::mat4 getViewMatrix() const {return viewMatrix;}
    float getZoom() const {return zoom;}
    glm::vec2 getPosition() const {return position;}
    GLfloat getCameraSpeed(GLfloat speed) const {return speed * deltaTime / zoom;}
    void setDeltaPosition(glm::vec2 previewPosition,GLfloat x,GLfloat y){
        position.x = previewPosition.x + x;
        position.y = previewPosition.y + y;
        updateViewMatrix();
    }
    void setExtent(Extent extent);
    void updateProjectionMatrix(GLfloat width,GLfloat height);
    void updateViewMatrix() ;
private:
    Camera2D();
    glm::vec2 position;
    float zoom;
    glm::mat4 projectionMatrix,viewMatrix;
    const GLfloat marginRate = 1.1;
    const GLfloat deltaTime = 0.016f; // 60FPS
};

#endif /* camera_hpp */
