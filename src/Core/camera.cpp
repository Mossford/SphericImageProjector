#include "camera.hpp"

Camera::Camera()
{

}

Camera::Camera(glm::vec3 position, glm::vec3 rotation, glm::vec3 target, float fov, int width, int height, float near, float far)
{
    this->position = position;
    this->rotation = rotation;
    this->target = target;
    this->fov = fov;
    this->width = width;
    this->height = height;
    this->near = near;
    this->far = far;
}

glm::vec3 Camera::GetCameraDir()
{
    float degToRad = M_PI / 180.0f;
    target.x = -sin(rotation.x * degToRad) * cos(rotation.y * degToRad);
    target.y = -sin(rotation.y * degToRad);
    target.z = cos(rotation.x * degToRad) * cos(rotation.y * degToRad);
    return glm::normalize(target);
}

glm::vec3 Camera::GetCameraUp()
{
    return glm::cross(GetCameraDir(), glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), GetCameraDir())));
}

glm::mat4 Camera::GetViewMat()
{
    glm::mat4 mat;
    if(lockTarget)
        mat = glm::lookAt(position, target, GetCameraUp());
    else
        mat = glm::lookAt(position, position + GetCameraDir(), GetCameraUp());
    lockTarget = false;
    return mat;
}

glm::mat4 Camera::GetProjMat()
{
    float degToRad = M_PI / 180.0f;
    return glm::perspective(fov * degToRad, (float)width / (float)height, near, far);
}

void Camera::LookAtPos(glm::vec3 pos)
{
    lockTarget = true;
    target = pos;
}
