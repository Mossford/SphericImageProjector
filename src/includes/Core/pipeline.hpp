#pragma once

#include <SDL3/SDL.h>
#include "shader.hpp"
#include "vertex.hpp"

struct AppContext;

class Pipeline
{
private:
    ShaderSettings vertSettings;
    ShaderSettings fragSettings;

public:

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo;
    SDL_GPUGraphicsPipeline* pipeline;
    Shader vertexShader;
    Shader fragmentShader;

    Pipeline();
    void Initalize(ShaderSettings vertSettings, ShaderSettings fragSettings);
    void CreatePipeline(AppContext* context);
    void Bind(SDL_GPURenderPass* renderPass);
    void Delete(AppContext* context);
};
