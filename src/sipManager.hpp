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
    //(hours in seconds) + (minutes in seconds) + seconds in utc starting at 0-24 hours
    float baseTime;

    //the last added image index
    int lastImage;
    int currentImageCount;

    //roation in degress per second
    const float earthRotationSpeed = 0.00382388888f * 60 * 60;
    //base latitude of images taken (will have to be adjusted so that each image will have its own and that will move where it should be, from the base)
    const float latitude = 45;

    SIPManager();
    void Initalize(AppContext* context, int maxImages, float baseTime);
    void Update(AppContext* context, float deltaTime);
    void Draw(AppContext* context, Camera* camera, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void DeleteImage(AppContext* context);
    void Clean(AppContext* context);
};
