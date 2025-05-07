#pragma once

#include <SDL3/SDL.h>
#include <imgui.h>
#include "pipeline.hpp"
#include "texture.hpp"
#include "sipManager.hpp"
#include "lineRenderer.hpp"

const Uint32 windowStartWidth = 1920;
const Uint32 windowStartHeight = 1080;

struct AppContext
{
    SDL_Window* window;
    Uint32 width;
    Uint32 height;
    SDL_GPUDevice* gpuDevice;
    const char* api;
    const char* gpuName;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
    const char* basePath;
    SDL_GPUViewport viewPort;
    Texture frameTexture;
    Texture depthTexture;
    Pipeline defaultPipeline;
    ImGuiIO* imguiIO;
    Mesh worldScreen;

    SIPManager sipManager;
    LineRenderer lineRenderer;
};
