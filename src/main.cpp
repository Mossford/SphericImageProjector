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
Mesh ground;
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

	if (!TTF_Init())
	{
		return SDL_FailCustom();
	}

	context.basePath = SDL_GetBasePath();

	context.gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);

	if (context.gpuDevice == NULL)
	{
		SDL_Log("GPUCreateDevice failed");
		return SDL_FailCustom();
	}

	context.window = SDL_CreateWindow("SphericImageProjector", windowStartWidth, windowStartHeight, SDL_WINDOW_RESIZABLE);
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

	std::string fontPath = std::string(context.basePath) + "Inter-VariableFont.ttf";
	TTF_Font* font = TTF_OpenFont(fontPath.c_str(), 36);
	if (!font)
	{
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

	m51.LoadFromFile(&context, "uvCheck.jpg");

	ground = Create2DQuad(glm::vec3(0,-0.05f,0), glm::vec3(90, 0, 0));
	ground.scale = glm::vec3(3);
	ground.CreateSmoothNormals();
	ground.BufferGens(&context);

	camera = Camera(glm::vec3(0,0,0), glm::vec3(0), glm::vec3(0, 0, 0), 70, windowStartWidth, windowStartHeight, 0.1f, 10000.0f);

	std::string north = "North";
	std::string east = "East";
	std::string south = "South";
	std::string west = "West";

	sipManager.Initalize(&context, 100, 0);
	sipManager.LoadImage(TTF_RenderText_Solid(font, north.data(), north.length(), { 255,255,255 }), 180, 0, glm::vec2(2,2), -1, &context);
	sipManager.LoadImage(TTF_RenderText_Solid(font, east.data(), east.length(), { 255,255,255 }), 270, 0, glm::vec2(2,2), -1, &context);
	sipManager.LoadImage(TTF_RenderText_Solid(font, south.data(), south.length(), { 255,255,255 }), 0, 0, glm::vec2(2,2), -1, &context);
	sipManager.LoadImage(TTF_RenderText_Solid(font, west.data(), west.length(), { 255,255,255 }), 90, 0, glm::vec2(2,2), -1, &context);
	sipManager.LoadImage("M51.png", 74.76f, 71.7f, glm::vec2(3.41f, 2.28f), 31730, &context);
	sipManager.LoadImage("M101.jpg", 54.55f, 67.85f, glm::vec2(3.41f, 2.28f), 32934, &context);
	sipManager.LoadImage("uvCheck.jpg", 90, 90, glm::vec2(1.0f, 1.0f), 32934, &context);

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
			if(event.window.type == SDL_EVENT_WINDOW_RESIZED)
			{
				int width, height;
				SDL_GetWindowSize(context.window, &width, &height);
				camera.width = width;
				camera.height = height;

				//there is probably a better way
				context.backBuffer.Delete(&context);
				context.backBuffer.CreateTexture(&context, SDL_GPU_TEXTURETYPE_2D, width, height, SDL_GPU_TEXTUREFORMAT_D16_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET);
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
	ground.rotation.z += sipManager.earthRotationSpeed * frameTime;

	sipManager.Update(&context, frameTime);

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
		colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.02f, 0.05f, 1.0f };
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
		
		ground.CreateModelMat();
		ground.DrawMesh(&context, renderPass, cmdbuf, proj, view);

		sipManager.Draw(&context, &camera, renderPass, cmdbuf);

		ImguiDraw(cmdbuf, renderPass);

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void ImguiUpdate()
{
	float curTime = SDL_GetTicks();

	static ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::Begin("SpaceTesting", nullptr, window_flags);

    //ImGui::Text("Version %s", EngVer.c_str());
    ImGui::Text("%.3f ms/frame (%.1f FPS)", (1.0f / context.imguiIO->Framerate) * 1000.0f, context.imguiIO->Framerate);
    //ImGui::Text("%u verts, %u indices (%u tris)", vertCount, indCount, indCount / 3);
    //ImGui::Text("DrawCall Avg: (%.1f) DC/frame, DrawCall Total (%d)", drawCallAvg, DrawCallCount);
    ImGui::Text("Time Open %.0f:%.2d", floorf(curTime / (60.0f * 1000.0f)), (int)(curTime / 1000.0f) % 60);
    //ImGui::Text("Time taken for Update run %.2fms ", fabs(updateTime));
    //ImGui::Text("Time taken for Fixed Update run %.2fms ", fabs(updateFixedTime));

	ImGui::Spacing();
	ImGui::Text("Number of loaded images: %d", sipManager.currentImageCount);

	float totalTime = sipManager.baseTime;
	int utcHours = floor(totalTime / 10000);
	int utcMins = (int)floor(totalTime / 100) % 100;
	int utcSeconds = (int)totalTime % 100;

	int utcHoursCur = (int)(floorf(((curTime / 1000.0f + utcSeconds) / 60.0f + utcMins) / 60.0f) + utcHours);
	int utcMinsCur = (int)(floorf((curTime / 1000.0f + utcSeconds) / 60.0f) + utcMins) % 60;
	int utcSecondsCur = ((int)(curTime / 1000.0f + utcSeconds) % 60);

	ImGui::Text("Base Time (UTC): %02d:%02d:%02d", utcHours, utcMins, utcSeconds);
	ImGui::Text("Time From Base (UTC): %02d:%02d:%02d", utcHoursCur, utcMinsCur, utcSecondsCur);

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
	ground.Delete(&context);

	sipManager.Clean(&context);

	context.defaultPipeline.Delete(&context);
    SDL_ReleaseWindowFromGPUDevice(context.gpuDevice, context.window);
    SDL_DestroyWindow(context.window);
    SDL_DestroyGPUDevice(context.gpuDevice);
    SDL_Quit();
}
