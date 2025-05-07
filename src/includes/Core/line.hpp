#pragma once
#include "pipeline.hpp"

struct AppContext;

class Line
{
private:
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;

public:

    glm::vec3 start;
    glm::vec3 end;

    glm::vec3 colorStart;
    glm::vec3 colorEnd;

    const Uint32 indices[6] = {0, 1, 2, 2, 1, 3};

    Line();
    void Create(AppContext* context, glm::vec3 start, glm::vec3 end, glm::vec3 startColor, glm::vec3 endColor, float thickness);
    void Draw(AppContext* context, Pipeline* pipeline, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void Delete(AppContext* context);
};
