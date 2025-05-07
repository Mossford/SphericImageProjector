#include "lineRenderer.hpp"

LineRenderer::LineRenderer()
{
    maxLines = 0;
    currentLineCount = 0;
}


void LineRenderer::Initalize(AppContext* context, int maxLines)
{
    this->maxLines = maxLines;
    lines = new Line[maxLines];

    pipeline.Initalize(ShaderSettings("line.vert", 0, 1, 0, 0), ShaderSettings("line.frag", 0, 0, 0, 0));
    pipeline.CreatePipeline(context, CreateDefaultLineAttributes(), sizeof(glm::vec3) * 2, 2);
}

void LineRenderer::AddLine(AppContext* context, glm::vec3 start, glm::vec3 end, glm::vec3 startColor, glm::vec3 endColor, float thickness)
{
    if(currentLineCount < maxLines)
    {
        lines[currentLineCount].Create(context, start, end, startColor, endColor, thickness);
        currentLineCount++;
    }
}

void LineRenderer::Draw(AppContext* context, Camera* camera, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf)
{
    glm::mat4 cameraMat = camera->GetProjMat() * camera->GetViewMat();

    pipeline.Bind(renderPass);

    for(int i = 0; i < currentLineCount; i++)
    {
        glm::vec3 pos = (lines[i].start + lines[i].end) / 2.0f;
        pipeline.vertexShader.AddMat4(cameraMat);
        pipeline.vertexShader.BindVertexUniformData(cmbBuf, 0);
        lines[i].Draw(context, &pipeline, renderPass, cmbBuf);
    }

    for(int i = 0; i < currentLineCount; i++)
    {
        lines[i].Delete(context);
    }

    currentLineCount = 0;
}

void LineRenderer::Clean(AppContext* context)
{
    pipeline.Delete(context);
    delete[] lines;
}


