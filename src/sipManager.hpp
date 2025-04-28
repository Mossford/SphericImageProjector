#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "sipImage.hpp"

struct AppContext;

class SIPManager
{
public:

    Uint32 maxImages;
    SIPImage* images;
    //the base time off which all other images can calculate movement
    float baseTime;

    //roation in degress per second
    const float earthRotationSpeed = 0.00382388888f;

    SIPManager();
    void Initalize(int maxImages, float baseTime);
    void LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void DeleteImage(AppContext* context);
};
