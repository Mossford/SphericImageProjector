#pragma once

#include <SDL3/SDL.h>
#include <imgui.h>
#include "pipeline.hpp"
#include "texture.hpp"
#include "sipManager.hpp"

const Uint32 windowStartWidth = 1920;
const Uint32 windowStartHeight = 1080;

struct AppContext
{
    SDL_Window* window;
    SDL_GPUDevice* gpuDevice;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
    const char* basePath;
    SDL_GPUViewport viewPort;
    Texture backBuffer;
    Pipeline defaultPipeline;
    ImGuiIO* imguiIO;

    SIPManager sipManager;
};
