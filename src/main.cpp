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
#include "camera.hpp"

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
Camera camera;
float xMouse = 0;
float yMouse = 0;
bool lockMouse = false;

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

	context.window = SDL_CreateWindow("SphericImageProjector", windowStartWidth, windowStartHeight, 0);
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

	SDL_GPUShader* vertexShader = CompileShaderProgram(context.basePath, "default.vert", context.gpuDevice, 0, 1, 0, 0, true);
	SDL_GPUShader* fragmentShader = CompileShaderProgram(context.basePath, "default.frag", context.gpuDevice, 0, 0, 0, 0, true);

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
	targetInfo.has_depth_stencil_target = true;
	targetInfo.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;

	SDL_GPUDepthStencilState depthStencil = {};
	depthStencil.enable_depth_test = true;
	depthStencil.enable_depth_write = true;
	depthStencil.enable_stencil_test = false;
	depthStencil.compare_op = SDL_GPU_COMPAREOP_LESS;
	depthStencil.write_mask = 0xFF;

	SDL_GPURasterizerState rasterizerState = {};
	rasterizerState.cull_mode = SDL_GPU_CULLMODE_NONE;
	rasterizerState.fill_mode = SDL_GPU_FILLMODE_FILL;
	rasterizerState.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.vertex_shader = vertexShader;
	pipelineCreateInfo.fragment_shader = fragmentShader;
	pipelineCreateInfo.vertex_input_state = vertexInputState;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineCreateInfo.target_info = targetInfo;
	pipelineCreateInfo.depth_stencil_state = depthStencil;
	pipelineCreateInfo.rasterizer_state = rasterizerState;

	context.pipeline.fillPipeline = SDL_CreateGPUGraphicsPipeline(context.gpuDevice, &pipelineCreateInfo);
	if (!context.pipeline.fillPipeline)
	{
		SDL_Log("Failed to create pipeline!");
		return SDL_FailCustom();
	}


	SDL_ReleaseGPUShader(context.gpuDevice, vertexShader);
	SDL_ReleaseGPUShader(context.gpuDevice, fragmentShader);


	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureInfo.width = windowStartWidth;
	textureInfo.height = windowStartHeight;
	textureInfo.layer_count_or_depth = 1;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

	context.pipeline.backBuffer = SDL_CreateGPUTexture(context.gpuDevice, &textureInfo);


	mesh = CreateSphereMesh(glm::vec3(0,0,0), glm::vec3(0,0,0), 4);
	mesh.BufferGens(&context);

	camera = Camera(glm::vec3(0,0,-10), glm::vec3(0), glm::vec3(0), 70);

	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				Quit();
				quit = true;
			}
			if(event.type == SDL_EVENT_MOUSE_MOTION)
			{
				xMouse = event.motion.xrel;
				yMouse = event.motion.yrel;
			}
			if(event.type == SDL_EVENT_KEY_DOWN)
			{
				if(event.key.key == SDLK_ESCAPE)
				{
					lockMouse = !lockMouse;
					SDL_SetWindowRelativeMouseMode(context.window, lockMouse);
				}
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
	mesh.rotation = glm::vec3(sin(SDL_GetTicks() / 50.0f), cos(SDL_GetTicks() / 50.0f), 0);
	camera.position = glm::vec3(sin(SDL_GetTicks() / 1000.0f) * 5, 0, cos(SDL_GetTicks() / 1000.0f) * 5);
	camera.LookAtPos(glm::vec3(0));

	if(lockMouse)
	{
		camera.rotation.x += xMouse * 1.5f;
		camera.rotation.y += yMouse * 1.5f;
		if(camera.rotation.y > 89.0f)
			camera.rotation.y =  89.0f;
		if(camera.rotation.y < -89.0f)
			camera.rotation.y = -89.0f;
	}


	xMouse = 0;
	yMouse = 0;
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
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.window, &swapchainTexture, NULL, NULL))
	{
		SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		SDL_FailCustom();
		return;
	}

	if (swapchainTexture != NULL)
	{
		SDL_GPUColorTargetInfo colorTargetInfo = {};
		colorTargetInfo.texture = swapchainTexture;
		colorTargetInfo.clear_color = (SDL_FColor){ 0.4f, 0.7f, 0.8f, 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {};
		depthStencilTargetInfo.texture = context.pipeline.backBuffer;
        depthStencilTargetInfo.cycle = true;
        depthStencilTargetInfo.clear_depth = true;
        depthStencilTargetInfo.clear_stencil = true;
        depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthStencilTargetInfo);
		SDL_BindGPUGraphicsPipeline(renderPass, context.pipeline.fillPipeline);
		
		mesh.DrawMesh(&context, renderPass, cmdbuf, camera.GetProjMat(windowStartWidth, windowStartHeight, 0.1f, 10000.0f), camera.GetViewMat());

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}


void Quit()
{
	SDL_ReleaseGPUTexture(context.gpuDevice, context.pipeline.backBuffer);
	SDL_ReleaseGPUGraphicsPipeline(context.gpuDevice, context.pipeline.fillPipeline);
	mesh.Delete(&context);
    SDL_ReleaseWindowFromGPUDevice(context.gpuDevice, context.window);
    SDL_DestroyWindow(context.window);
    SDL_DestroyGPUDevice(context.gpuDevice);
    SDL_Quit();
}
