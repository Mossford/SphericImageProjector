#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <string_view>
#include <filesystem>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include "app.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "vertex.hpp"

void Update();
void Draw();
void Init();
void Quit();
bool quit;

int SDL_FailCustom()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_FailCustom();
}

AppContext context;
Mesh mesh;

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return SDL_FailCustom();
	}

	context.basePath = SDL_GetBasePath();

	context.gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);

	if (context.gpuDevice == NULL)
	{
		SDL_Log("GPUCreateDevice failed");
		return SDL_FailCustom();
	}

	context.window = SDL_CreateWindow("SphericImageProjector", 1920, 1080, 0);
	if (context.window == NULL)
	{
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return SDL_FailCustom();
	}

	if (!SDL_ClaimWindowForGPUDevice(context.gpuDevice, context.window))
	{
		SDL_Log("GPUClaimWindow failed");
		return SDL_FailCustom();
	}

	// Create the shaders
	glslang_initialize_process();

	SDL_GPUShader* vertexShader = CompileShaderProgram(context.basePath, "default2.vert", context.gpuDevice, 0, 0, 0, 0);
	SDL_GPUShader* fragmentShader = CompileShaderProgram(context.basePath, "default2.frag", context.gpuDevice, 0, 0, 0, 0);

	glslang_finalize_process();

	// Create the pipeline
	SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
	vertexBufferDescriptions[0] = {};
	vertexBufferDescriptions[0].slot = 0;
	vertexBufferDescriptions[0].pitch = sizeof(Vertex);
	vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertexBufferDescriptions[0].instance_step_rate = 0;

	SDL_GPUVertexAttribute vertexAttributes[3];
	vertexAttributes[0] = {};
	vertexAttributes[0].location = 0;
	vertexAttributes[0].buffer_slot = 0;
	vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttributes[0].offset = 0;

	vertexAttributes[1] = {};
	vertexAttributes[1].location = 1;
	vertexAttributes[1].buffer_slot = 0;
	vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttributes[1].offset = offsetof(Vertex, normal);

	vertexAttributes[2] = {};
	vertexAttributes[2].location = 2;
	vertexAttributes[2].buffer_slot = 0;
	vertexAttributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	vertexAttributes[2].offset = offsetof(Vertex, uv);

	SDL_GPUVertexInputState vertexInputState = {};
	vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions;
	vertexInputState.num_vertex_buffers = 1;
	vertexInputState.vertex_attributes = vertexAttributes;
	vertexInputState.num_vertex_attributes = 3;

	SDL_GPUColorTargetDescription colorTargetDescriptions[1];
	colorTargetDescriptions[0] = {};
	colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(context.gpuDevice, context.window);

	SDL_GPUGraphicsPipelineTargetInfo targetInfo = {};
	targetInfo.color_target_descriptions = colorTargetDescriptions;
	targetInfo.num_color_targets = 1;

	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.vertex_shader = vertexShader;
	pipelineCreateInfo.fragment_shader = fragmentShader;
	pipelineCreateInfo.vertex_input_state = vertexInputState;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineCreateInfo.target_info = targetInfo;

	context.pipeline.fillPipeline = SDL_CreateGPUGraphicsPipeline(context.gpuDevice, &pipelineCreateInfo);
	if (!context.pipeline.fillPipeline)
	{
		SDL_Log("Failed to create pipeline!");
		return SDL_FailCustom();
	}


	SDL_ReleaseGPUShader(context.gpuDevice, vertexShader);
	SDL_ReleaseGPUShader(context.gpuDevice, fragmentShader);

	mesh = CreateSphereMesh(glm::vec3(0,0,0), glm::vec3(0,0,0), 4);
	mesh.BufferGens(&context);


	while (!quit)
	{
		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_EVENT_QUIT)
			{
				Quit();
				quit = true;
			}
		}
		if (quit)
		{
			break;
		}
		
		Update();
		Draw();
		}

	return 0;
}

void Update()
{

}

void Draw()
{
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.gpuDevice);
	if (cmdbuf == NULL)
	{
		SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
		SDL_FailCustom();
		return;
	}

	SDL_GPUTexture* swapchainTexture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.window, &swapchainTexture, NULL, NULL)) {
		SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		SDL_FailCustom();
		return;
	}

	if (swapchainTexture != NULL)
	{
		SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
		colorTargetInfo.texture = swapchainTexture;
		colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
		SDL_BindGPUGraphicsPipeline(renderPass, context.pipeline.fillPipeline);
		
		mesh.DrawMesh(&context, renderPass);

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}


void Quit()
{
	SDL_ReleaseGPUGraphicsPipeline(context.gpuDevice, context.pipeline.fillPipeline);
	mesh.Delete(&context);
    SDL_ReleaseWindowFromGPUDevice(context.gpuDevice, context.window);
    SDL_DestroyWindow(context.window);
    SDL_DestroyGPUDevice(context.gpuDevice);
    SDL_Quit();
}
