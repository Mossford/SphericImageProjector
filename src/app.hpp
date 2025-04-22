#pragma once

#include <SDL3/SDL.h>
#include "shader.hpp"

const Uint32 windowStartWidth = 1920;
const Uint32 windowStartHeight = 1080;

struct GpuPipeline
{
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo;
    SDL_GPUGraphicsPipeline* fillPipeline;
    SDL_GPUViewport viewPort;
    Shader vertexShader;
    Shader fragmentShader;
};

struct AppContext
{
    SDL_Window* window;
    SDL_GPUDevice* gpuDevice;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
    const char* basePath;
    GpuPipeline pipeline;
};