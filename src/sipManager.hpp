#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "sipImage.hpp"
#include "pipeline.hpp"
#include "camera.hpp"

struct AppContext;

class SIPManager
{
public:

    Uint32 maxImages;
    SIPImage* images;
    Pipeline pipeline;
    //the base time off which all other images can calculate movement
    float baseTime;

    //the last added image index
    int lastImage;
    int currentImageCount;

    //roation in degress per second
    const float earthRotationSpeed = 0.00382388888f;

    SIPManager();
    void Initalize(AppContext* context, int maxImages, float baseTime);
    void Update(AppContext* context, float deltaTime);
    void Draw(AppContext* context, Camera* camera, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void DeleteImage(AppContext* context);
    void Clean(AppContext* context);
};
