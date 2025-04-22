#pragma once

#include <SDL3/SDL.h>

struct AppContext
{
    SDL_Window* window;
    SDL_GPUDevice* gpuDevice;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
    const char* basePath;
    GpuPipeline pipeline;
};

struct GpuPipeline
{
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo;
    SDL_GPUGraphicsPipeline* fillPipeline;
};