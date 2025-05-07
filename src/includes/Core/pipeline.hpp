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
    /**
     * Sets values to the pipelines values
     * @param vertSettings The vertex shader settings
     * @param fragSettings The fragment shader settings
     */
    void Initalize(ShaderSettings vertSettings, ShaderSettings fragSettings);
    /**
     * Creates the pipeline to be used with the shaders in the pipeline object
     * Vertex Attributes array expects to be on heap
     * @param context The app context
     * @param vertexAttrib Array of vertex attributes
     * @param pitch The sizeof the vertex
     * @param vertexAttribCount Number of vertex Attributes
     */
    void CreatePipeline(AppContext* context, SDL_GPUVertexAttribute* vertexAttrib, Uint32 pitch, int vertexAttribCount);
    /**
     * Binds the pipeline to be used
     * @param renderPass The current renderpass
     */
    void Bind(SDL_GPURenderPass* renderPass);
    /**
     * Deletes the pipeline
     * @param context The app context
     */
    void Delete(AppContext* context);
};

/**
 * Creates an array of vertexAttributes that are the default (position, normal, uv)
 * Needs to be deleted after use
 * @return heap allocated array of vertexAtrributes
 */
SDL_GPUVertexAttribute* CreateDefaultVertAttributes();
/**
 * Creates an array of vertexAttributes that are the default for lines (position, color)
 * Needs to be deleted after use
 * @return heap allocated array of vertexAtrributes
 */
SDL_GPUVertexAttribute* CreateDefaultLineAttributes();
