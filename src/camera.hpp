#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Camera
{
public:

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 target;
    float fov;
    bool lockTarget;

    Camera();
    Camera(glm::vec3 position, glm::vec3 rotation, glm::vec3 target, float fov);
    glm::vec3 GetCameraDir();
    glm::vec3 GetCameraUp();
    glm::mat4 GetViewMat();
    glm::mat4 GetProjMat(int width, int height, float near, float far);
    void LookAtPos(glm::vec3 pos);
};