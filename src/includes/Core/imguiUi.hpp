#pragma once
#include <imgui.h>
#include <SDL3/SDL.h>
#include <string>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlgpu3.h>

struct AppContext;

void MainImguiMenu(AppContext* context);
void DrawImgui(SDL_GPUCommandBuffer* cmdBuf, SDL_GPURenderPass* renderPass);
