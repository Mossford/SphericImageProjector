#include "sipCamera.hpp"
#include "app.hpp"

SIPCamera::SIPCamera()
{
    accumCount = 0;
    accumResetTime = 0;
    accumTime = 0;
    frameMix = 0;
}

void SIPCamera::Initalize(AppContext* context, float accumResetTime)
{
    this->accumResetTime = accumResetTime;

    accumFrame.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, windowStartWidth, windowStartHeight, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);
    accumFrame.CreateSampler(context, CreateDefaultPixelSampler());

    tempRenderTexture.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, windowStartWidth, windowStartHeight, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);

    quad = Create2DQuad(glm::vec3(0.0f), glm::vec3(0.0f));
    quad.BufferGens(context);

    pipeline.Initalize(ShaderSettings("SIPCamera.vert", 0, 1, 0, 0), ShaderSettings("SIPCamera.frag", 2, 1, 0, 0));
    pipeline.CreatePipeline(context, CreateDefaultVertAttributes(), sizeof(Vertex), 3);

    frameMix = 0.01f;
}

void SIPCamera::Update(AppContext* context, float dt)
{
    accumTime += dt;
    accumCount++;

    if(accumTime >= accumResetTime)
    {
        accumTime = 0.0f;
        accumCount = 0;

        CopyTexture(context, &context->frameTexture, &accumFrame);
        CopyTexture(context, &context->frameTexture, &tempRenderTexture);
    }
}

void SIPCamera::Reset(AppContext* context)
{
    accumFrame.Delete(context);
    accumFrame.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, context->width, context->height, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);
    accumFrame.CreateSampler(context, CreateDefaultPixelSampler());

    tempRenderTexture.Delete(context);
    tempRenderTexture.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, context->width, context->height, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);

    CopyTexture(context, &context->frameTexture, &accumFrame);
    CopyTexture(context, &context->frameTexture, &tempRenderTexture);
}


void SIPCamera::Render(AppContext* context, SDL_GPUCommandBuffer* cmdbuf)
{
    SDL_GPUColorTargetInfo colorTargetInfo = {};
    colorTargetInfo.texture = tempRenderTexture.texture;
    colorTargetInfo.clear_color = (SDL_FColor){ 0.2f, 0.2f, 0.2f, 0.0f };
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    pipeline.Bind(renderPass);

    Texture* textures[2] = {&accumFrame, &context->frameTexture};
    BindTextures(renderPass, 0, textures, 2);
    pipeline.fragmentShader.AddFloat(frameMix);
    pipeline.fragmentShader.BindFragmentUniformData(cmdbuf, 0);
    pipeline.vertexShader.AddMat4(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f)));
    pipeline.vertexShader.BindVertexUniformData(cmdbuf, 0);
    quad.DrawMesh(context, renderPass, cmdbuf);

    SDL_EndGPURenderPass(renderPass);

    CopyTexture(context, &tempRenderTexture, &accumFrame);

}

void SIPCamera::Clean(AppContext* context)
{
    accumFrame.Delete(context);
    quad.Delete(context);
    pipeline.Delete(context);
}

