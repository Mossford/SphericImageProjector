#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_image/SDL_image.h>
#include <string>

struct AppContext;

class Texture
{
public:
    SDL_GPUTexture* texture;
    SDL_GPUSampler* sampler;
    std::string file;
    SDL_GPUTextureType type;
    int width;
    int height;
    SDL_GPUTextureFormat format;
    SDL_GPUTextureUsageFlags usage;
    SDL_GPUTextureSamplerBinding samplerBinding;

    Texture();
    Texture(const Texture &other);
    void CreateTexture(AppContext* context, SDL_GPUTextureType type, int width, int height, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage);
    void CreateSampler(AppContext* context, SDL_GPUSamplerCreateInfo samplerInfo);
    void LoadFromFile(AppContext* context, std::string file);
    void LoadFromLocation(AppContext* context, std::string location);
    void CreateFromSurface(AppContext* context, SDL_Surface* image);
    void BindSampler(SDL_GPURenderPass* renderPass, int slot);
    void Delete(AppContext* context);
};
