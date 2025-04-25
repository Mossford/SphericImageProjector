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
Mesh mesh2;
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


	context.defaultPipeline.Initalize(ShaderSettings("default.vert", 0, 1, 0, 0), ShaderSettings("default.frag", 0, 0, 0, 0));
	context.defaultPipeline.CreatePipeline(&context);


	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureInfo.width = windowStartWidth;
	textureInfo.height = windowStartHeight;
	textureInfo.layer_count_or_depth = 1;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

	context.backBuffer = SDL_CreateGPUTexture(context.gpuDevice, &textureInfo);


	mesh = CreateSphereMesh(glm::vec3(3,0,0), glm::vec3(0,0,0), 3);
	mesh.scale = 10;
	mesh.CreateSmoothNormals();
	mesh.BufferGens(&context);
	mesh2 = CreateSphereMesh(glm::vec3(3,0,0), glm::vec3(0,0,0), 3);
	mesh2.CreateSmoothNormals();
	mesh2.BufferGens(&context);

	camera = Camera(glm::vec3(0,0,0), glm::vec3(0), glm::vec3(0), 70);

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
	mesh.rotation = glm::vec3(sin(SDL_GetTicks() / 50.0f) * 3, cos(SDL_GetTicks() / 50.0f) * 3, 0);
	//camera.position = glm::vec3(sin(SDL_GetTicks() / 1000.0f) * 0.5f, 0, cos(SDL_GetTicks() / 1000.0f) * 0.5f);
	//camera.LookAtPos(glm::vec3(0));

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
		depthStencilTargetInfo.texture = context.backBuffer;
        depthStencilTargetInfo.cycle = true;
        depthStencilTargetInfo.clear_depth = true;
        depthStencilTargetInfo.clear_stencil = true;
        depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthStencilTargetInfo);
		SDL_BindGPUGraphicsPipeline(renderPass, context.defaultPipeline.pipeline);

		glm::mat4 proj = camera.GetProjMat(windowStartWidth, windowStartHeight, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMat();
		
		mesh.DrawMesh(&context, renderPass, cmdbuf, proj, view);
		mesh2.DrawMesh(&context, renderPass, cmdbuf, proj, view);

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}


void Quit()
{
	SDL_ReleaseGPUTexture(context.gpuDevice, context.backBuffer);
	SDL_ReleaseGPUGraphicsPipeline(context.gpuDevice, context.defaultPipeline.pipeline);
	mesh.Delete(&context);
	mesh2.Delete(&context);
    SDL_ReleaseWindowFromGPUDevice(context.gpuDevice, context.window);
    SDL_DestroyWindow(context.window);
    SDL_DestroyGPUDevice(context.gpuDevice);
    SDL_Quit();
}
