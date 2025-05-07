#pragma once
#include <glm/glm.hpp>
#include <string>
#include "texture.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"

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
    bool applyTilt;
    glm::vec3 rotation;

    Texture image;
    Mesh mesh;

    SIPImage();
    void CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void CreateFromLocation(std::string location, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void CreateFromSurface(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context);
    void CreateFromLocation(std::string location, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context);
    void CreateFromSurface(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context);
    void UpdateMesh(AppContext* context);
    void DrawMesh(AppContext* context, Pipeline* pipeline, glm::mat4 proj, glm::mat4 view, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void Delete(AppContext* context);

    //applys the earths rotation
    void ApplyRotation(float earthRotation, float earthOrbit, float latitude, float deltaTime, float time);
    glm::vec3 GetPosition();
    glm::vec3 ProjectPosToImage(AppContext* context, glm::vec3 pos);
};
