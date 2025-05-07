#pragma once
#include "texture.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"

struct AppContext;

class SIPCamera
{
private:
    float accumTime;
    Texture tempRenderTexture;
    int accumCount;

public:

    Mesh quad;
    //store accumulated frames into a texture
    Texture accumFrame;
    //the time at which the accumulation frame is reset
    float accumResetTime;
    Pipeline pipeline;
    //the amount to mix between the past and current frame
    float frameMix;


    SIPCamera();
    void Initalize(AppContext* context, float accumResetTime);
    void Update(AppContext* context, float dt);
    /**
     * Will render to a texture stored in the object which will be accumulated over time
     */
    void Render(AppContext* context, SDL_GPUCommandBuffer* cmdbuf);
    void Reset(AppContext* context);
    void Clean(AppContext* context);
};

