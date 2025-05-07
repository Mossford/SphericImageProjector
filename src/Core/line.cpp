#include "line.hpp"
#include "app.hpp"

Line::Line()
{

}

void Line::Create(AppContext* context, glm::vec3 start, glm::vec3 end, glm::vec3 startColor, glm::vec3 endColor, float thickness)
{
    this->start = start;
    this->end = end;
    this->colorStart = startColor;
    this->colorEnd = endColor;

    Uint32 vertexSize = sizeof(glm::vec3) * 8;
    Uint32 indiceSize = sizeof(Uint32) * 6;

    SDL_GPUBufferCreateInfo bufferCreateInfoVert = {};
    bufferCreateInfoVert.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    bufferCreateInfoVert.size = vertexSize;

    SDL_GPUBufferCreateInfo bufferCreateInfoInd = {};
    bufferCreateInfoInd.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    bufferCreateInfoInd.size = indiceSize;

    vertexBuffer = SDL_CreateGPUBuffer(context->gpuDevice, &bufferCreateInfoVert);
    indexBuffer = SDL_CreateGPUBuffer(context->gpuDevice, &bufferCreateInfoInd);

    SDL_GPUTransferBufferCreateInfo bufferTransferInfo = {};
    bufferTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    bufferTransferInfo.size = vertexSize + indiceSize;

    // To get data into the vertex buffer, we have to use a transfer buffer
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(context->gpuDevice, &bufferTransferInfo);

    void* transferData = SDL_MapGPUTransferBuffer(context->gpuDevice, transferBuffer, false);

    glm::vec3* vertexData = (glm::vec3*)transferData;

    vertexData[0] = start + glm::vec3(0.0f, thickness, 0.0f);
    vertexData[1] = startColor;

    vertexData[2] = start - glm::vec3(0.0f, thickness, 0.0f);
    vertexData[3] = startColor;

    vertexData[4] = end + glm::vec3(0.0f, thickness, 0.0f);
    vertexData[5] = endColor;

    vertexData[6] = end - glm::vec3(0.0f, thickness, 0.0f);
    vertexData[7] = endColor;

    Uint32* indexData = (Uint32*) &vertexData[8];
    for (int i = 0; i < 6; i++)
    {
        indexData[i] = indices[i];
    }

    SDL_UnmapGPUTransferBuffer(context->gpuDevice, transferBuffer);

    // Upload the transfer data to the vertex buffer
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_GPUTransferBufferLocation bufferlocVert = {};
    bufferlocVert.transfer_buffer = transferBuffer;
    bufferlocVert.offset = 0;

    SDL_GPUBufferRegion bufferregionVert = {};
    bufferregionVert.buffer = vertexBuffer;
    bufferregionVert.offset = 0;
    bufferregionVert.size = vertexSize;

    SDL_GPUTransferBufferLocation bufferlocInd = {};
    bufferlocInd.transfer_buffer = transferBuffer;
    bufferlocInd.offset = vertexSize;

    SDL_GPUBufferRegion bufferregionInd = {};
    bufferregionInd.buffer = indexBuffer;
    bufferregionInd.offset = 0;
    bufferregionInd.size = indiceSize;

    SDL_UploadToGPUBuffer(copyPass, &bufferlocVert, &bufferregionVert, false);
    SDL_UploadToGPUBuffer(copyPass, &bufferlocInd, &bufferregionInd, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(context->gpuDevice, transferBuffer);
}

void Line::Draw(AppContext* context, Pipeline* pipeline, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf)
{
    SDL_GPUBufferBinding bufBindVert = {};
    bufBindVert.buffer = vertexBuffer;
    bufBindVert.offset = 0;
    SDL_BindGPUVertexBuffers(renderPass, 0, &bufBindVert, 1);
    SDL_GPUBufferBinding bufBindInd = {};
    bufBindInd.buffer = indexBuffer;
    bufBindInd.offset = 0;
    SDL_BindGPUIndexBuffer(renderPass, &bufBindInd, SDL_GPU_INDEXELEMENTSIZE_32BIT);
    SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);
}

void Line::Delete(AppContext* context)
{
    if(vertexBuffer != NULL)
        SDL_ReleaseGPUBuffer(context->gpuDevice, vertexBuffer);
    if(indexBuffer != NULL)
        SDL_ReleaseGPUBuffer(context->gpuDevice, indexBuffer);
}

