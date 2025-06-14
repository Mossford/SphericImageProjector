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
#include "imguiUi.hpp"
#include "sipManager.hpp"

void Update(AppContext* context);
void Draw(AppContext* context);

SDL_AppResult SDL_FailCustom()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

Mesh ground;
Camera camera;
Texture groundTexture;
float pastXMouse = 0;
float pastYMouse = 0;
float xMouse = 0;
float yMouse = 0;
float frameTime;
float pastTime;
bool lockMouse = false;

SDL_AppResult SDL_AppInit(void** app, int argc, char* argv[])
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

	//get a refrence so I can set the context from here
	*app = new AppContext();
	AppContext* context = (AppContext*)*app;

	context->basePath = SDL_GetBasePath();

	context->gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);

	if (context->gpuDevice == NULL)
	{
		SDL_Log("GPUCreateDevice failed");
		return SDL_FailCustom();
	}

	//This causes renderDoc to crash
	//context->api = SDL_GetGPUDeviceDriver(context->gpuDevice);
	//SDL_PropertiesID id;
	//id = SDL_GetGPUDeviceProperties(context->gpuDevice);
	//context->gpuName = SDL_GetStringProperty(id, SDL_PROP_GPU_DEVICE_NAME_STRING, "");

	context->window = SDL_CreateWindow("SphericImageProjector", windowStartWidth, windowStartHeight, SDL_WINDOW_RESIZABLE);
	if (context->window == NULL)
	{
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return SDL_FailCustom();
	}

	if (!SDL_ClaimWindowForGPUDevice(context->gpuDevice, context->window))
	{
		SDL_Log("GPUClaimWindow failed");
		return SDL_FailCustom();
	}
	if(SDL_WindowSupportsGPUPresentMode(context->gpuDevice, context->window, SDL_GPU_PRESENTMODE_IMMEDIATE))
	{
		SDL_SetGPUSwapchainParameters(context->gpuDevice, context->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);
	}

	std::string fontPath = std::string(context->basePath) + "Inter-VariableFont.ttf";
	TTF_Font* font = TTF_OpenFont(fontPath.c_str(), 36);
	if (!font)
	{
		return SDL_FailCustom();
	}

	//imgui initialize
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    context->imguiIO = &ImGui::GetIO();
    context->imguiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    context->imguiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForSDLGPU(context->window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = context->gpuDevice;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(context->gpuDevice, context->window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    ImGui_ImplSDLGPU3_Init(&init_info);

	SetImGuiStyle();

	context->defaultPipeline.Initalize(ShaderSettings("default.vert", 0, 1, 0, 0), ShaderSettings("default.frag", 1, 1, 0, 0));
	context->defaultPipeline.CreatePipeline(context, CreateDefaultVertAttributes(), sizeof(Vertex), 3);

	//allow the depth and render texture to be useable in shaders and whatever
	context->depthTexture.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, windowStartWidth, windowStartHeight, SDL_GPU_TEXTUREFORMAT_D16_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET);
	context->frameTexture.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, windowStartWidth, windowStartHeight, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);
	context->frameTexture.CreateSampler(context, CreateDefaultPixelSampler());
	context->depthTexture.CreateSampler(context, CreateDefaultPixelSampler());

	//create the quad to render the frame to
	context->worldScreen = Create2DQuad(glm::vec3(0.0f), glm::vec3(0.0f));
	context->worldScreen.BufferGens(context);

	context->lineRenderer.Initalize(context, 100);

	groundTexture.LoadFromFile(context, "uvCheck.jpg");

	ground = CreateCubeSphereMesh(glm::vec3(0,-51.5f,0), glm::vec3(0, 0, 0), 3);
	ground.scale = glm::vec3(1000, 50, 1000);
	ground.CreateSmoothNormals();
	ground.BufferGens(context);

	camera = Camera(glm::vec3(0,0,0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 70, windowStartWidth, windowStartHeight, 0.1f, 10000.0f);

	std::string north = "North";
	std::string east = "East";
	std::string south = "South";
	std::string west = "West";

	context->sipManager.Initalize(context, 100, 0);
	context->sipManager.LoadImage(TTF_RenderText_Solid(font, north.data(), north.length(), { 255,255,255 }), 0, 0, glm::vec2(2,2), -1, false, context);
	context->sipManager.LoadImage(TTF_RenderText_Solid(font, east.data(), east.length(), { 255,255,255 }), 90, 0, glm::vec2(2,2), -1, false, context);
	context->sipManager.LoadImage(TTF_RenderText_Solid(font, south.data(), south.length(), { 255,255,255 }), 180, 0, glm::vec2(2,2), -1, false, context);
	context->sipManager.LoadImage(TTF_RenderText_Solid(font, west.data(), west.length(), { 255,255,255 }), 270, 0, glm::vec2(2,2), -1, false, context);
	context->sipManager.LoadImage("M51.png", 74.76f, 71.7f, glm::vec2(3.41f, 2.28f), 31730, true, context);
	context->sipManager.LoadImage("M101.jpg", 54.55f, 67.85f, glm::vec2(3.41f, 2.28f), 32934, true, context);
	context->sipManager.LoadImage("uvCheck.jpg", 0, 45, glm::vec2(1.0f, 1.0f), 31730, context);

	/*for(int i = 0; i < 360; i += 10)
	{
		context->sipManager.LoadImage("uvCheck.jpg", i, 45, glm::vec2(1.0f, 1.0f), 31730, true, &context);
	}*/

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event)
{
	AppContext* context = (AppContext*)appstate;

	ImGui_ImplSDL3_ProcessEvent(event);
	if (event->type == SDL_EVENT_QUIT)
	{
		context->app_quit = SDL_APP_SUCCESS;
	}
	if(event->type == SDL_EVENT_MOUSE_WHEEL)
	{
		if(!ImGui::GetIO().WantCaptureMouse || lockMouse)
		{
			camera.fov -= camera.fov * 0.1 * event->wheel.y;
			camera.fov = std::min(170.0f, camera.fov);
		}
	}
	if(event->type == SDL_EVENT_KEY_DOWN)
	{
		if(event->key.key == SDLK_ESCAPE)
		{
			pastXMouse = xMouse;
			pastYMouse = yMouse;
			lockMouse = !lockMouse;
			SDL_SetWindowRelativeMouseMode(context->window, lockMouse);
		}
	}
	if(event->window.type == SDL_EVENT_WINDOW_RESIZED)
	{
		int width, height;
		SDL_GetWindowSize(context->window, &width, &height);
		camera.width = width;
		camera.height = height;
		context->width = width;
		context->height = height;

		//there is probably a better way
		context->depthTexture.Delete(context);
		context->depthTexture.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, width, height, SDL_GPU_TEXTUREFORMAT_D16_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET);
		context->frameTexture.Delete(context);
		context->frameTexture.CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, width, height, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);
		context->frameTexture.CreateSampler(context, CreateDefaultPixelSampler());
		context->depthTexture.CreateSampler(context, CreateDefaultPixelSampler());
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	AppContext* context = (AppContext*)appstate;

	float time = SDL_GetTicks();
	frameTime = time - pastTime;
	frameTime /= 1000.0f;
	SDL_GetRelativeMouseState(&xMouse, &yMouse);

	Update(context);
	Draw(context);

	pastTime = time;

	return context->app_quit;
}

void Update(AppContext* context)
{
	ground.rotation.y -= context->sipManager.earthRotationSpeed * frameTime * context->sipManager.speed;

	context->sipManager.sipCamera.Update(context, frameTime);

	context->sipManager.Update(context, frameTime);

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

void Draw(AppContext* context)
{

	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->gpuDevice);
	if (cmdbuf == NULL)
	{
		SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
		SDL_FailCustom();
		return;
	}

	SDL_GPUTexture* swapChainTexture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context->window, &swapChainTexture, NULL, NULL))
	{
		SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		SDL_FailCustom();
		return;
	}

	//main render pass
	if (swapChainTexture != NULL)
	{
		SDL_GPUColorTargetInfo colorTargetInfo = {};
		colorTargetInfo.texture = context->frameTexture.texture;
		colorTargetInfo.clear_color = (SDL_FColor){ abs(0.0f - context->sipManager.GetDayNightCycle()), abs(0.02f - context->sipManager.GetDayNightCycle()), abs(0.05f - context->sipManager.GetDayNightCycle()), 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {};
		depthStencilTargetInfo.texture = context->depthTexture.texture;
        depthStencilTargetInfo.cycle = true;
        depthStencilTargetInfo.clear_depth = true;
        depthStencilTargetInfo.clear_stencil = false;
        depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;


		//render the world

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthStencilTargetInfo);
		context->defaultPipeline.Bind(renderPass);
		groundTexture.BindSampler(renderPass, 0);

		glm::mat4 proj = camera.GetProjMat();
		glm::mat4 view = camera.GetViewMat();

		ground.CreateModelMat();
		glm::mat4 combineMat = proj * view * ground.modelMatrix;
		context->defaultPipeline.vertexShader.AddMat4(combineMat);
		context->defaultPipeline.vertexShader.BindVertexUniformData(cmdbuf, 0);
		context->defaultPipeline.fragmentShader.AddFloat(context->sipManager.GetDayNightCycle());
		context->defaultPipeline.fragmentShader.BindFragmentUniformData(cmdbuf, 0);
		ground.DrawMesh(context, renderPass, cmdbuf);

		context->sipManager.Draw(context, &camera, renderPass, cmdbuf);
		context->lineRenderer.Draw(context, &camera, renderPass, cmdbuf);

		SDL_EndGPURenderPass(renderPass);

		context->sipManager.sipCamera.Render(context, cmdbuf);

		//extra render stuff like displaying world

		colorTargetInfo = {};
		colorTargetInfo.texture = swapChainTexture;
		colorTargetInfo.clear_color = (SDL_FColor){ 0.2f, 0.2f, 0.2f, 0.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

		//bind the image pipeline as it takes uvs and puts an image on
		context->sipManager.pipeline.Bind(renderPass);

		//draw the rendered frame
		context->frameTexture.BindSampler(renderPass, 0);
		context->sipManager.pipeline.vertexShader.AddMat4(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f)));
		context->sipManager.pipeline.vertexShader.BindVertexUniformData(cmdbuf, 0);
		context->worldScreen.DrawMesh(context, renderPass, cmdbuf);

		ImGui_ImplSDLGPU3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		MainImguiMenu(context);
		DrawImgui(cmdbuf, renderPass);

		SDL_EndGPURenderPass(renderPass);

	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	AppContext* context = (AppContext*)appstate;

	if(context)
	{
		groundTexture.Delete(context);
		context->depthTexture.Delete(context);
		context->frameTexture.Delete(context);
		ground.Delete(context);
		context->worldScreen.Delete(context);

		context->sipManager.Clean(context);
		context->lineRenderer.Clean(context);

		context->defaultPipeline.Delete(context);
		SDL_ReleaseWindowFromGPUDevice(context->gpuDevice, context->window);
		SDL_DestroyWindow(context->window);
		SDL_DestroyGPUDevice(context->gpuDevice);

		delete context;
	}

	TTF_Quit();
	SDL_Quit();
}
