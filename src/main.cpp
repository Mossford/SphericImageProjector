#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <string_view>
#include <filesystem>

#include "app.hpp"
#include "mesh.hpp"

constexpr uint32_t windowStartWidth = 1920;
constexpr uint32_t windowStartHeight = 1080;

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
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
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

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo =
    {
		.target_info =
        {
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[])
            {{
				.format = SDL_GetGPUSwapchainTextureFormat(gpuDevice, window)
			}},
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vertexShader,
		.fragment_shader = fragmentShader,
	};

    *appstate = new AppContext
    {
       .window = window,
       .gpuDevice = gpuDevice,
       .basePath = basePath.string().c_str(),
    };

    SDL_SetRenderVSync(renderer, -1);

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
    

    return app->app_quit;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    AppContext* app = (AppContext*)appstate;
    if (app)
    {
        SDL_ReleaseWindowFromGPUDevice(app->gpuDevice, app->window);
        SDL_DestroyWindow(app->window);
        SDL_DestroyGPUDevice(app->gpuDevice);
        delete app;
    }
    
    SDL_Quit();
}
