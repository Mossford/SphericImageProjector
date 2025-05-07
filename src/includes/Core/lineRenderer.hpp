#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "pipeline.hpp"
#include "camera.hpp"
#include "line.hpp"

struct AppContext;

class LineRenderer
{

public:

    int maxLines;
    Line* lines;
    Pipeline pipeline;

    //will be reset after each draw so lines get overwritten
    int currentLineCount;

    LineRenderer();
    void Initalize(AppContext* context, int maxlines);
    void AddLine(AppContext* context, glm::vec3 start, glm::vec3 end, glm::vec3 startColor, glm::vec3 endColor, float thickness);
    void Draw(AppContext* context, Camera* camera, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void Clean(AppContext* context);
};
