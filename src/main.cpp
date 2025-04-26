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
    return SDL_FailCustom();
}

AppContext context;
Mesh mesh;
Mesh mesh2;
Camera camera;
Texture m51;
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

	m51.LoadFromFile(&context, "M51.png");

	mesh = CreateSphereMesh(glm::vec3(3,0,0), glm::vec3(0,0,0), 3);
	mesh.scale = 2;
	mesh.GenerateSphereUv();
	mesh.CreateSmoothNormals();
	mesh.BufferGens(&context);
	mesh2 = CreateSphereMesh(glm::vec3(-3,0,0), glm::vec3(0,0,0), 3);
	mesh2.scale = 2;
	mesh2.GenerateSphereUv();
	mesh2.CreateSmoothNormals();
	mesh2.BufferGens(&context);

	camera = Camera(glm::vec3(0,0,-3), glm::vec3(0), glm::vec3(3, 0, 0), 70);

	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
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
	mesh.rotation = glm::vec3(0, SDL_GetTicks() / 500.0f, 0);
	mesh2.rotation = glm::vec3(SDL_GetTicks() / 500.0f, 0, 0);
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

		glm::mat4 proj = camera.GetProjMat(windowStartWidth, windowStartHeight, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMat();
		
		mesh.DrawMesh(&context, renderPass, cmdbuf, proj, view);
		mesh2.DrawMesh(&context, renderPass, cmdbuf, proj, view);

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

    //needs to be io.framerate because the actal deltatime is polled too fast and the 
    //result is hard to read
    //ImGui::Text("Version %s", EngVer.c_str());
    ImGui::Text("%.3f ms/frame (%.1f FPS)", (1.0f / context.imguiIO->Framerate) * 1000.0f, context.imguiIO->Framerate);
    //ImGui::Text("%u verts, %u indices (%u tris)", vertCount, indCount, indCount / 3);
    //ImGui::Text("DrawCall Avg: (%.1f) DC/frame, DrawCall Total (%d)", drawCallAvg, DrawCallCount);
    ImGui::Text("Time Open %.1f minutes", (SDL_GetTicks() / (60.0f * 1000.0f)));
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
	mesh.Delete(&context);
	mesh2.Delete(&context);
	context.defaultPipeline.Delete(&context);
    SDL_ReleaseWindowFromGPUDevice(context.gpuDevice, context.window);
    SDL_DestroyWindow(context.window);
    SDL_DestroyGPUDevice(context.gpuDevice);
    SDL_Quit();
}
