#pragma once
#include <glm/glm.hpp>
#include <string>
#include "texture.hpp"
#include "mesh.hpp"

struct AppContext;

class SIPImage
{
public:

    std::string file;
    float azimuth;
    float altitude;
    glm::vec2 angularSize;
    //time the image was taken
    float time;
    bool created;
    glm::vec3 rotation;

    Texture image;
    Mesh mesh;

    SIPImage();
    void CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void UpdateMesh(AppContext* context);
    void DrawMesh(AppContext* context, glm::mat4 proj, glm::mat4 view, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void Delete(AppContext* context);

    //applys the earths rotation
    void ApplyRotation(float rotiation, float deltaTime);
};
