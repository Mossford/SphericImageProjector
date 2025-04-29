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
    int width;
    int height;
    float near;
    float far;
    bool lockTarget;

    Camera();
    Camera(glm::vec3 position, glm::vec3 rotation, glm::vec3 target, float fov, int width, int height, float near, float far);
    glm::vec3 GetCameraDir();
    glm::vec3 GetCameraUp();
    glm::mat4 GetViewMat();
    glm::mat4 GetProjMat();
    void LookAtPos(glm::vec3 pos);
};
