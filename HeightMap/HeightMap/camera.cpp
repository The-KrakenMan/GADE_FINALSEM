#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>


//Camera::Camera() 
//{
//    // Initialize camera properties
//    positions[0] = glm::vec3(0.0f, 1.0f, 3.0f);  // First position
//    positions[1] = glm::vec3(0.0f, 2.0f, 5.0f);  // Second position
//    positions[2] = glm::vec3(0.0f, 3.0f, 7.0f);  // Third position
//    currentPosition = 0; // Start at the first position
//}
//
//void Camera::setPosition(int positionIndex)
//{
//    if (positionIndex >= 0 && positionIndex < 3) {
//        currentPosition = positionIndex;
//    }
//}
//
//void Camera::cyclePosition(int direction)
//{ // Wrap around the cameras
//    currentPosition += direction;
//
//    // Wrap around to the first position if at the end
//    if (currentPosition >= 3) {
//        currentPosition = 0;
//    }
//    // Wrap around to the last position if at the beginning
//    else if (currentPosition < 0) {
//        currentPosition = 2;
//    }
//}
//
//void Camera::lookAt(const glm::vec3& target) 
//{
//    front = glm::normalize(target - positions[currentPosition]);
//    up = glm::vec3(0.0f, 1.0f, 0.0f);
//}
//
//glm::mat4 Camera::getViewMatrix() const
//{
//    glm::mat4 viewMatrix = glm::lookAt(positions[currentPosition], positions[currentPosition] + front, up);  ////shows where camera is looking
//    return viewMatrix;
//}
//
//glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
//{
//
//}