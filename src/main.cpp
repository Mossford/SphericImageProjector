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
#include "mesh.hpp"
#include "vertex.hpp"

void Draw(AppContext* app);
SDL_GPUBuffer* VertexBuffer;

SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        return SDL_Fail();
    }
    
    if (!TTF_Init())
    {
        return SDL_Fail();
    }
   
    SDL_Window* window = SDL_CreateWindow("SphericImageProjector", windowStartWidth, windowStartHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window)
    {
        return SDL_Fail();
    }

    SDL_GPUDevice* gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    if(!gpuDevice)
    {
        return SDL_Fail();
    }
    
    const char* basePathPtr = SDL_GetBasePath();
    if (!basePathPtr)
    {
        return SDL_Fail();
    }
    const std::filesystem::path basePath = basePathPtr;

    const auto fontPath = basePath / "Inter-VariableFont.ttf";
    TTF_Font* font = TTF_OpenFont(fontPath.string().c_str(), 36);
    if (!font)
    {
        return SDL_Fail();
    }
    TTF_CloseFont(font);

    if(!SDL_ClaimWindowForGPUDevice(gpuDevice, window))
    {
        return SDL_Fail();
    }
    
    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
    }

    glslang_initialize_process();

    GpuPipeline gpuPipeline;
    gpuPipeline.vertexShader.CompileShader(basePath.string() + "default2.vert", gpuDevice, 0, 0, 0, 0);
    gpuPipeline.fragmentShader.CompileShader(basePath.string() + "default2.frag", gpuDevice, 0, 0, 0, 0);

    glslang_finalize_process();

    gpuPipeline.pipelineCreateInfo =
    {
        .vertex_shader = gpuPipeline.vertexShader.shader,
		.fragment_shader = gpuPipeline.fragmentShader.shader,
        .vertex_input_state = (SDL_GPUVertexInputState){
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[])
            {{
				.slot = 0,
                .pitch = sizeof(float) * 3,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
			}},
			.num_vertex_buffers = 1,
			.vertex_attributes = (SDL_GPUVertexAttribute[])
            {{
                .location = 0,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = 0
			}},
            .num_vertex_attributes = 1,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info =
        {
            .color_target_descriptions = (SDL_GPUColorTargetDescription[])
            {{
				.format = SDL_GetGPUSwapchainTextureFormat(gpuDevice, window)
			}},
			.num_color_targets = 1,
		},
	};

    gpuPipeline.pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL,

    gpuPipeline.viewPort = {0, 0, windowStartWidth, windowStartHeight, 0.1f, 1.0f};

    gpuPipeline.fillPipeline = SDL_CreateGPUGraphicsPipeline(gpuDevice, &gpuPipeline.pipelineCreateInfo);
	if (!gpuPipeline.fillPipeline)
	{
		return SDL_Fail();
	}

    *appstate = new AppContext
    {
       .window = window,
       .gpuDevice = gpuDevice,
       .basePath = basePath.string().c_str(),
       .pipeline = gpuPipeline,
    };

    SDL_ReleaseGPUShader(gpuDevice, gpuPipeline.fragmentShader.shader);
    SDL_ReleaseGPUShader(gpuDevice, gpuPipeline.vertexShader.shader);

    SDL_GPUBufferCreateInfo bufferCreateInfo = {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(glm::vec3) * 3};
    SDL_GPUTransferBufferCreateInfo bufferTransferInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = sizeof(glm::vec3) * 3};

    VertexBuffer = SDL_CreateGPUBuffer(gpuDevice, &bufferCreateInfo);
    if(!VertexBuffer)
    {
        return SDL_Fail();
    }

	// To get data into the vertex buffer, we have to use a transfer buffer
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &bufferTransferInfo);

	glm::vec3* transferData = (glm::vec3*)SDL_MapGPUTransferBuffer(gpuDevice, transferBuffer, false);

	transferData[0] = glm::vec3(-1, -1, 0);
	transferData[1] = glm::vec3(1, -1, 0);
	transferData[2] = glm::vec3(0, 1, 0);

	SDL_UnmapGPUTransferBuffer(gpuDevice, transferBuffer);

	// Upload the transfer data to the vertex buffer
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_GPUTransferBufferLocation bufferTransferLocation = {.transfer_buffer = transferBuffer, .offset = 0};
    SDL_GPUBufferRegion bufferRegion = {.buffer = VertexBuffer, .offset = 0, .size = sizeof(glm::vec3) * 3};

	SDL_UploadToGPUBuffer(copyPass, &bufferTransferLocation, &bufferRegion, false);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(gpuDevice, transferBuffer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event)
 {
    AppContext* app = (AppContext*)appstate;
    
    if (event->type == SDL_EVENT_QUIT)
    {
        app->app_quit = SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    AppContext* app = (AppContext*)appstate;
    
    Draw(app);

    return app->app_quit;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    AppContext* app = (AppContext*)appstate;
    if (app)
    {
        SDL_ReleaseGPUGraphicsPipeline(app->gpuDevice, app->pipeline.fillPipeline);
        SDL_ReleaseWindowFromGPUDevice(app->gpuDevice, app->window);
        SDL_DestroyWindow(app->window);
        SDL_DestroyGPUDevice(app->gpuDevice);
        delete app;
    }
    
    SDL_Quit();
}

void Draw(AppContext* app)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(app->gpuDevice);
    if (cmdbuf == NULL)
    {
        SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, app->window, &swapchainTexture, NULL, NULL))
    {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        return;
    }

	if (swapchainTexture != NULL)
	{
		SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
		colorTargetInfo.texture = swapchainTexture;
		colorTargetInfo.clear_color = (SDL_FColor){ 1.0f, 0.0f, 0.0f, 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

        SDL_GPUBufferBinding bufferBinding = { .buffer = VertexBuffer, .offset = 0 };

		SDL_BindGPUGraphicsPipeline(renderPass, app->pipeline.fillPipeline);
		SDL_BindGPUVertexBuffers(renderPass, 0, &bufferBinding, 1);
		SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}
