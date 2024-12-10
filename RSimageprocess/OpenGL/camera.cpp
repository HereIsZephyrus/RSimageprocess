//
//  camera.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/10/24.
//

#include "camera.hpp"
#include "window.hpp"
#include "commander.hpp"
void Camera2D::processKeyboard(GLFWwindow* window) {
    BufferRecorder& buffer = BufferRecorder::getBuffer();
    const GLfloat cameraSpeed = getCameraSpeed(4.0f);
    if (buffer.pressAlt || buffer.pressCtrl|| buffer.pressShift)//ignore function input
        return;
    if (buffer.keyRecord[GLFW_KEY_W])
        position.y += cameraSpeed;
    if (buffer.keyRecord[GLFW_KEY_S])
        position.y -= cameraSpeed;
    if (buffer.keyRecord[GLFW_KEY_A])
        position.x -= cameraSpeed;
    if (buffer.keyRecord[GLFW_KEY_D])
        position.x += cameraSpeed;
    updateViewMatrix();
}
void Camera2D::processScroll(GLFWwindow* window, double xoffset, double yoffset, bool pressCtrl, bool pressAlt){
    const GLfloat cameraSpeed = getCameraSpeed(4.0f);
    if (pressCtrl)//if conflict, ctrl first:yscroll
        position.y += cameraSpeed * yoffset;
    else if (pressAlt)//if conflict, then alt:xscroll
        position.x -= cameraSpeed * yoffset;
    else{
        zoomInOut(static_cast<float>(yoffset));
    }
    updateViewMatrix();
}
void Camera2D::zoomInOut(float yOffset) {
    zoom -= yOffset * 0.1f;
    if (zoom < 0.01f)
        zoom = 0.01f;
    if (zoom > 100.0f)
        zoom = 100.0f;
}
Camera2D::Camera2D() : position(0.0f, 0.0f), zoom(1.0f){
    updateProjectionMatrix();
    updateViewMatrix();
}
void Camera2D::updateProjectionMatrix(){
    projectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f);
}
void Camera2D::updateViewMatrix() {
    glm::vec3 scaling(zoom, zoom, 1.0f);
    viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
    viewMatrix = glm::scale(viewMatrix, scaling);
}

