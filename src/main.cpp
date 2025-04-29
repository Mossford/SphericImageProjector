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
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlgpu3.h>

#include "app.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "vertex.hpp"
#include "camera.hpp"
#include "sipManager.hpp"

void Update();
void Draw();
void Init();
void Quit();
void ImguiUpdate();
void ImguiDraw(SDL_GPUCommandBuffer* cmdBuf, SDL_GPURenderPass* renderPass);
bool quit;

int SDL_FailCustom()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return -1;
}

AppContext context;
Mesh sphere;
Camera camera;
Texture m51;
float pastXMouse = 0;
float pastYMouse = 0;
float xMouse = 0;
float yMouse = 0;
float frameTime;
float pastTime;
bool lockMouse = false;

SIPManager sipManager;

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
	if(SDL_WindowSupportsGPUPresentMode(context.gpuDevice, context.window, SDL_GPU_PRESENTMODE_IMMEDIATE))
	{
		//SDL_SetGPUSwapchainParameters(context.gpuDevice, context.window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);
	}

	//imgui initialize
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    context.imguiIO = &ImGui::GetIO();
    context.imguiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    context.imguiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForSDLGPU(context.window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = context.gpuDevice;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(context.gpuDevice, context.window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    ImGui_ImplSDLGPU3_Init(&init_info);

	context.defaultPipeline.Initalize(ShaderSettings("default.vert", 0, 1, 0, 0), ShaderSettings("default.frag", 1, 0, 0, 0));
	context.defaultPipeline.CreatePipeline(&context);

	context.backBuffer.CreateTexture(&context, SDL_GPU_TEXTURETYPE_2D, windowStartWidth, windowStartHeight, SDL_GPU_TEXTUREFORMAT_D16_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET);

	m51.LoadFromFile(&context, "uvCheck.jpg");

	sphere = CreateSphereMesh(glm::vec3(0,0,0), glm::vec3(0), 3);
	sphere.scale = glm::vec3(1.1f);
	sphere.CreateSmoothNormals();
	sphere.BufferGens(&context);

	camera = Camera(glm::vec3(0,0,0), glm::vec3(0), glm::vec3(0, 0, 0), 70, windowStartWidth, windowStartHeight, 0.1f, 10000.0f);

	sipManager.Initalize(&context, 100, 0);
	sipManager.LoadImage("M51.png", 40, 50, glm::vec2(0.2, 0.1), 0, &context);

	while (!quit)
	{
		float time = SDL_GetTicks();
		frameTime = time - pastTime;
		frameTime /= 1000.0f;

		SDL_GetRelativeMouseState(&xMouse, &yMouse);
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
			{
				Quit();
				quit = true;
			}
			if(event.type == SDL_EVENT_MOUSE_WHEEL)
			{
				camera.fov -= camera.fov * 0.1 * event.wheel.y;
				camera.fov = std::min(170.0f, camera.fov);
			}
			if(event.type == SDL_EVENT_KEY_DOWN)
			{
				if(event.key.key == SDLK_ESCAPE)
				{
					pastXMouse = xMouse;
					pastYMouse = yMouse;
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

		pastTime = time;
	}

	return 0;
}

void Update()
{
	float earthRotationSpeed = 0.00382388888f * 3600.0f;
	sphere.rotation.x = 23.4f;
	sphere.rotation.y += earthRotationSpeed * frameTime;

	if(lockMouse)
	{
		camera.rotation.x += xMouse * 0.25f * camera.fov * 0.01;
		camera.rotation.y += yMouse * 0.25f * camera.fov * 0.01;
		if(camera.rotation.y > 89.0f)
			camera.rotation.y =  89.0f;
		if(camera.rotation.y < -89.0f)
			camera.rotation.y = -89.0f;
		pastXMouse = xMouse;
		pastYMouse = yMouse;
	}
}

void Draw()
{
	ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImguiUpdate();

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
		depthStencilTargetInfo.texture = context.backBuffer.texture;
        depthStencilTargetInfo.cycle = true;
        depthStencilTargetInfo.clear_depth = true;
        depthStencilTargetInfo.clear_stencil = true;
        depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthStencilTargetInfo);
		context.defaultPipeline.Bind(renderPass);
		m51.BindSampler(renderPass, 0);

		glm::mat4 proj = camera.GetProjMat();
		glm::mat4 view = camera.GetViewMat();
		
		sphere.CreateModelMat();
		sphere.DrawMesh(&context, renderPass, cmdbuf, proj, view);

		sipManager.Draw(&context, &camera, renderPass, cmdbuf);

		ImguiDraw(cmdbuf, renderPass);

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void ImguiUpdate()
{
	static ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::Begin("SpaceTesting", nullptr, window_flags);

    //ImGui::Text("Version %s", EngVer.c_str());
    ImGui::Text("%.3f ms/frame (%.1f FPS)", (1.0f / context.imguiIO->Framerate) * 1000.0f, context.imguiIO->Framerate);
    //ImGui::Text("%u verts, %u indices (%u tris)", vertCount, indCount, indCount / 3);
    //ImGui::Text("DrawCall Avg: (%.1f) DC/frame, DrawCall Total (%d)", drawCallAvg, DrawCallCount);
    ImGui::Text("Time Open %.0f:%.2d", floorf(SDL_GetTicks() / (60.0f * 1000.0f)), (int)(SDL_GetTicks() / 1000.0f) % 60);
    //ImGui::Text("Time taken for Update run %.2fms ", fabs(updateTime));
    //ImGui::Text("Time taken for Fixed Update run %.2fms ", fabs(updateFixedTime));

    ImGui::Spacing();
    ImGui::DragFloat3("Player Position", glm::value_ptr(camera.position), 0.1f, -10.0f, 10.0f);
    ImGui::DragFloat3("Player Rotation", glm::value_ptr(camera.rotation), 1.0f, -360.0f, 360.0f);
    ImGui::SliderFloat("Cam Fov", &camera.fov, 179.9f, 0.01f);
	ImGui::End();
}

void ImguiDraw(SDL_GPUCommandBuffer* cmdBuf, SDL_GPURenderPass* renderPass)
{
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, cmdBuf);
    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, cmdBuf, renderPass);
}

void Quit()
{
	m51.Delete(&context);
	context.backBuffer.Delete(&context);
	sphere.Delete(&context);

	sipManager.Clean(&context);

	context.defaultPipeline.Delete(&context);
    SDL_ReleaseWindowFromGPUDevice(context.gpuDevice, context.window);
    SDL_DestroyWindow(context.window);
    SDL_DestroyGPUDevice(context.gpuDevice);
    SDL_Quit();
}
