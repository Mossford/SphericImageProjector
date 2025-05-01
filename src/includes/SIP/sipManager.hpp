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
    //the time adjusted with the speed in seconds
    double time;
    float speed;

    //the last added image index
    int lastImage;
    int currentImageCount;

    //roation in degress per second (from 15 degrees per hour)
    const float earthRotationSpeed = 0.00382388888f;
    //rotation in degrees per second of the orbit amount
    const float earthOrbitSpeed = 0.00001141552f;
    //base latitude of images taken (will have to be adjusted so that each image will have its own and that will move where it should be, from the base)
    const float latitude = -45;
    //the distance of the images
    const float radius = 1000.0f;

    SIPManager();
    void Initalize(AppContext* context, int maxImages, float baseTime);
    void Update(AppContext* context, float deltaTime);
    void Draw(AppContext* context, Camera* camera, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void LoadImageAbsolute(std::string location, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void LoadImage(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context);
    void LoadImageAbsolute(std::string location, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context);
    void LoadImage(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context);
    void DeleteImage(AppContext* context);
    void Clean(AppContext* context);
};
