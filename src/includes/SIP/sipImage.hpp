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
    //(hours * 10000) + (minutes * 100) + seconds in utc starting at 0-24 hours
    float time;
    bool created;
    glm::vec3 rotation;

    Texture image;
    Mesh mesh;

    SIPImage();
    void CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, float earthRotation, AppContext* context);
    void CreateFromSurface(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, float earthRotation, AppContext* context);
    void UpdateMesh(AppContext* context);
    void DrawMesh(AppContext* context, glm::mat4 proj, glm::mat4 view, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void Delete(AppContext* context);

    //applys the earths rotation
    void ApplyRotation(float earthRotation, float latitude, float deltaTime);
};
