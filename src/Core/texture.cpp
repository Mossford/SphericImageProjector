#include "texture.hpp"
#include "app.hpp"

Texture::Texture()
{

}

Texture::Texture(const Texture& other)
{
    this->file = other.file;
    this->format = other.format;
    this->height = other.height;
    this->width = other.width;
    this->sampler = other.sampler;
    this->texture = other.texture;
    this->type = other.type;
    this->usage = other.usage;
}


void Texture::CreateTexture(AppContext* context, SDL_GPUTextureType type, int width, int height, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage)
{

    this->type = type;
    this->width = width;
    this->height = height;
    this->format = format;
    this->usage = usage;

    SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = type;
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.layer_count_or_depth = 1;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	textureInfo.format = format;
	textureInfo.usage = usage;

	texture = SDL_CreateGPUTexture(context->gpuDevice, &textureInfo);

    samplerBinding.texture = texture;
}

void Texture::CreateSampler(AppContext* context, SDL_GPUSamplerCreateInfo samplerInfo)
{
    sampler = SDL_CreateGPUSampler(context->gpuDevice, &samplerInfo);
    samplerBinding.sampler = sampler;
}

void Texture::LoadFromFile(AppContext* context, std::string file)
{
    SDL_Surface* image = IMG_Load((context->basePath + file).c_str());
    if(image == NULL)
    {
        unsigned char pixels[64][64][3];
        SDL_Log("Could not find image %s", file.c_str());
        image = SDL_CreateSurface(64, 64, SDL_PIXELFORMAT_RGB24);
        for (unsigned int x = 0; x < 64; x++)
        {
            for (unsigned int y = 0; y < 64; y++)
            {
                if ((x / 4 + y / 4) % 2 == 0)
                {
                    pixels[x][y][0] = 255;
                    pixels[x][y][1] = 0;
                    pixels[x][y][2] = 255;
                }
                else
                {
                    pixels[x][y][0] = 0;
                    pixels[x][y][1] = 0;
                    pixels[x][y][2] = 0;
                }
            }
        }

        image->pixels = &pixels;
    }
    //flip vertical as sdl loads it vertically flipped
    SDL_FlipSurface(image, SDL_FLIP_VERTICAL);
    //flip horizontal
    SDL_FlipSurface(image, SDL_FLIP_HORIZONTAL);

    CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, image->w, image->h, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER);
    SDL_GPUSamplerCreateInfo samplerInfo = {};
    samplerInfo.min_filter = SDL_GPU_FILTER_NEAREST;
	samplerInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
	samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    CreateSampler(context, samplerInfo);

    int pixelSize = 4;
    //if the format is not 32 bit then give an error
    if(image->format != SDL_PIXELFORMAT_ABGR8888)
    {
        SDL_Surface* convert = SDL_ConvertSurface(image, SDL_PIXELFORMAT_ABGR8888);
		SDL_DestroySurface(image);
		image = convert;
    }

    SDL_GPUTransferBufferCreateInfo bufferTransferInfo = {};
	bufferTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	bufferTransferInfo.size = image->w * image->h * pixelSize;

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(context->gpuDevice, &bufferTransferInfo);

	Uint8* textureTransferPtr = (Uint8*)SDL_MapGPUTransferBuffer(context->gpuDevice, textureTransferBuffer, false);
    Uint8* data = (Uint8*)image->pixels;

    for (unsigned int i = 0; i < image->w * image->h * pixelSize; i++)
    {
        textureTransferPtr[i] = data[i];
    }
    
	SDL_UnmapGPUTransferBuffer(context->gpuDevice, textureTransferBuffer);

	// Upload the transfer data to the GPU resources
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->gpuDevice);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_GPUTextureTransferInfo textureTransferInfo = {};
    textureTransferInfo.transfer_buffer = textureTransferBuffer;
    textureTransferInfo.offset = 0;

    SDL_GPUTextureRegion textureRegion = {};
    textureRegion.texture = texture;
    textureRegion.w = image->w;
    textureRegion.h = image->h;
    textureRegion.d = 1;

    SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

    this->file = file;
    this->width = image->w;
    this->height = image->h;

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_DestroySurface(image);
	SDL_ReleaseGPUTransferBuffer(context->gpuDevice, textureTransferBuffer);

    SDL_GPUTextureSamplerBinding samplerBind = {};
    samplerBind.texture = texture;
    samplerBind.sampler = sampler;
    samplerBinding = samplerBind;
}

void Texture::LoadFromLocation(AppContext* context, std::string location)
{
    SDL_Surface* image = IMG_Load(location.c_str());
    if(image == NULL)
    {
        unsigned char pixels[64][64][3];
        SDL_Log("Could not find image %s", file.c_str());
        image = SDL_CreateSurface(64, 64, SDL_PIXELFORMAT_RGB24);
        for (unsigned int x = 0; x < 64; x++)
        {
            for (unsigned int y = 0; y < 64; y++)
            {
                if ((x / 4 + y / 4) % 2 == 0)
                {
                    pixels[x][y][0] = 255;
                    pixels[x][y][1] = 0;
                    pixels[x][y][2] = 255;
                }
                else
                {
                    pixels[x][y][0] = 0;
                    pixels[x][y][1] = 0;
                    pixels[x][y][2] = 0;
                }
            }
        }

        image->pixels = &pixels;
    }
    //flip vertical as sdl loads it vertically flipped
    SDL_FlipSurface(image, SDL_FLIP_VERTICAL);
    //flip horizontal
    SDL_FlipSurface(image, SDL_FLIP_HORIZONTAL);

    CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, image->w, image->h, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER);
    SDL_GPUSamplerCreateInfo samplerInfo = {};
    samplerInfo.min_filter = SDL_GPU_FILTER_NEAREST;
    samplerInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    CreateSampler(context, samplerInfo);

    int pixelSize = 4;
    //if the format is not 32 bit then give an error
    if(image->format != SDL_PIXELFORMAT_ABGR8888)
    {
        SDL_Surface* convert = SDL_ConvertSurface(image, SDL_PIXELFORMAT_ABGR8888);
        SDL_DestroySurface(image);
        image = convert;
    }

    SDL_GPUTransferBufferCreateInfo bufferTransferInfo = {};
    bufferTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    bufferTransferInfo.size = image->w * image->h * pixelSize;

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(context->gpuDevice, &bufferTransferInfo);

    Uint8* textureTransferPtr = (Uint8*)SDL_MapGPUTransferBuffer(context->gpuDevice, textureTransferBuffer, false);
    Uint8* data = (Uint8*)image->pixels;

    for (unsigned int i = 0; i < image->w * image->h * pixelSize; i++)
    {
        textureTransferPtr[i] = data[i];
    }

    SDL_UnmapGPUTransferBuffer(context->gpuDevice, textureTransferBuffer);

    // Upload the transfer data to the GPU resources
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_GPUTextureTransferInfo textureTransferInfo = {};
    textureTransferInfo.transfer_buffer = textureTransferBuffer;
    textureTransferInfo.offset = 0;

    SDL_GPUTextureRegion textureRegion = {};
    textureRegion.texture = texture;
    textureRegion.w = image->w;
    textureRegion.h = image->h;
    textureRegion.d = 1;

    SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

    this->file = location;
    this->width = image->w;
    this->height = image->h;

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    SDL_DestroySurface(image);
    SDL_ReleaseGPUTransferBuffer(context->gpuDevice, textureTransferBuffer);

    SDL_GPUTextureSamplerBinding samplerBind = {};
    samplerBind.texture = texture;
    samplerBind.sampler = sampler;
    samplerBinding = samplerBind;
}


void Texture::CreateFromSurface(AppContext* context, SDL_Surface* image)
{
    if(image == NULL)
    {
        unsigned char pixels[64][64][3];
        SDL_Log("Could not find image %s", file.c_str());
        image = SDL_CreateSurface(64, 64, SDL_PIXELFORMAT_RGB24);
        for (unsigned int x = 0; x < 64; x++)
        {
            for (unsigned int y = 0; y < 64; y++)
            {
                if ((x / 4 + y / 4) % 2 == 0)
                {
                    pixels[x][y][0] = 255;
                    pixels[x][y][1] = 0;
                    pixels[x][y][2] = 255;
                }
                else
                {
                    pixels[x][y][0] = 0;
                    pixels[x][y][1] = 0;
                    pixels[x][y][2] = 0;
                }
            }
        }

        image->pixels = &pixels;
    }
    //flip vertical as sdl loads it vertically flipped
    SDL_FlipSurface(image, SDL_FLIP_VERTICAL);
    //flip horizontal
    SDL_FlipSurface(image, SDL_FLIP_HORIZONTAL);

    CreateTexture(context, SDL_GPU_TEXTURETYPE_2D, image->w, image->h, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER);
    SDL_GPUSamplerCreateInfo samplerInfo = {};
    samplerInfo.min_filter = SDL_GPU_FILTER_NEAREST;
    samplerInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    CreateSampler(context, samplerInfo);

    int pixelSize = 4;
    //if the format is not 32 bit then give an error
    if(image->format != SDL_PIXELFORMAT_ABGR8888)
    {
        SDL_Surface* convert = SDL_ConvertSurface(image, SDL_PIXELFORMAT_ABGR8888);
        SDL_DestroySurface(image);
        image = convert;
    }

    SDL_GPUTransferBufferCreateInfo bufferTransferInfo = {};
    bufferTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    bufferTransferInfo.size = image->w * image->h * pixelSize;

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(context->gpuDevice, &bufferTransferInfo);

    Uint8* textureTransferPtr = (Uint8*)SDL_MapGPUTransferBuffer(context->gpuDevice, textureTransferBuffer, false);
    Uint8* data = (Uint8*)image->pixels;

    for (unsigned int i = 0; i < image->w * image->h * pixelSize; i++)
    {
        textureTransferPtr[i] = data[i];
    }

    SDL_UnmapGPUTransferBuffer(context->gpuDevice, textureTransferBuffer);

    // Upload the transfer data to the GPU resources
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_GPUTextureTransferInfo textureTransferInfo = {};
    textureTransferInfo.transfer_buffer = textureTransferBuffer;
    textureTransferInfo.offset = 0;

    SDL_GPUTextureRegion textureRegion = {};
    textureRegion.texture = texture;
    textureRegion.w = image->w;
    textureRegion.h = image->h;
    textureRegion.d = 1;

    SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

    this->width = image->w;
    this->height = image->h;

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    SDL_DestroySurface(image);
    SDL_ReleaseGPUTransferBuffer(context->gpuDevice, textureTransferBuffer);

    SDL_GPUTextureSamplerBinding samplerBind = {};
    samplerBind.texture = texture;
    samplerBind.sampler = sampler;
    samplerBinding = samplerBind;
}

void Texture::BindSampler(SDL_GPURenderPass* renderPass, int slot)
{
    if(texture == NULL || sampler == NULL || samplerBinding.texture == NULL || samplerBinding.sampler == NULL)
    {
        SDL_Log("Texture or sampler was NULL or samplerbinding refs were NULL");
        return;
    }

    SDL_BindGPUFragmentSamplers(renderPass, slot, &samplerBinding, 1);
}

void Texture::Delete(AppContext* context)
{
    if(texture != NULL)
        SDL_ReleaseGPUTexture(context->gpuDevice, texture);
    if(sampler != NULL)
        SDL_ReleaseGPUSampler(context->gpuDevice, sampler);
}

SDL_GPUSamplerCreateInfo CreateDefaultPixelSampler()
{
    SDL_GPUSamplerCreateInfo samplerInfo = {};
    samplerInfo.min_filter = SDL_GPU_FILTER_NEAREST;
    samplerInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    return samplerInfo;
}

void BindTextures(SDL_GPURenderPass* renderPass, int slot, Texture** textures, int count)
{
    SDL_GPUTextureSamplerBinding* bindings = new SDL_GPUTextureSamplerBinding[count];
    for(int i = 0; i < count; i++)
    {
        if(textures[i]->texture == NULL || textures[i]->sampler == NULL)
        {
            SDL_Log("Texture or sampler was NULL");
            return;
        }
        SDL_GPUTextureSamplerBinding samplerBind = {};
        samplerBind.texture = textures[i]->texture;
        samplerBind.sampler = textures[i]->sampler;
        bindings[i] = samplerBind;
    }

    SDL_BindGPUFragmentSamplers(renderPass, slot, bindings, count);

    delete[] bindings;
}

void CopyTexture(AppContext* context, Texture* src, Texture* dest)
{
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(context->gpuDevice);

    SDL_GPUBlitInfo blitInfo = {};
    blitInfo.source.texture = src->texture;
    blitInfo.source.layer_or_depth_plane = 0;
    blitInfo.source.w = src->width;
    blitInfo.source.h = src->height;
    blitInfo.destination.texture = dest->texture;
    blitInfo.destination.layer_or_depth_plane = 0;
    blitInfo.destination.w = dest->width;
    blitInfo.destination.h = dest->height;
    blitInfo.load_op = SDL_GPU_LOADOP_DONT_CARE;
    blitInfo.filter = SDL_GPU_FILTER_LINEAR;

    SDL_BlitGPUTexture(cmdBuf, &blitInfo);

    SDL_SubmitGPUCommandBuffer(cmdBuf);
}
