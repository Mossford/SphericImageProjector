#include "shader.hpp"

Shader::Shader()
{

}

Shader::~Shader()
{
    
}

void Shader::CompileShader(std::string location, SDL_GPUDevice* device, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount)
{
    const char *filePath = location.c_str();

    SDL_GPUShaderStage stage;
	if (SDL_strstr(filePath, ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(filePath, ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		SDL_Log("Invalid shader stage!");
        return;
	}

	char fullPath[256];
	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
	//for hlsl this will only be main
	const char *entrypoint = "main";

	size_t codeSize;
	void* code = SDL_LoadFile(filePath, &codeSize);
	if (code == NULL)
	{
		SDL_Log("Failed to load shader from disk! %s", filePath);
		return;
	}

	SDL_GPUShaderCreateInfo shaderInfo =
	{
		.code_size = codeSize,
		.code = (Uint8*)code,
		.entrypoint = entrypoint,
		.format = format,
		.stage = stage,
		.num_samplers = samplerCount,
		.num_storage_textures = storageTextureCount,
		.num_storage_buffers = storageBufferCount,
		.num_uniform_buffers = uniformBufferCount,
		//extensions
		.props = 0
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL)
	{
		SDL_Log("Failed to create shader!");
		SDL_free(code);
		return;
	}

	SDL_free(code);
}   

void Shader::setBool(std::string name, bool value)
{         
    
}
void Shader::setInt(std::string name, int value)
{ 
    
}
void Shader::setFloat(std::string name, float value)
{ 
    
}
void Shader::setVec2(std::string name, glm::vec2 value)
{ 
    
}
void Shader::setVec2(std::string name, float x, float y)
{ 
    
}
void Shader::setVec3(std::string name, glm::vec3 value)
{ 
    
}
void Shader::setVec3(std::string name, float x, float y, float z)
{ 
    
}
void Shader::setVec4(std::string name, glm::vec4 value)
{ 
    
}
void Shader::setVec4(std::string name, float x, float y, float z, float w)
{ 
    
}
void Shader::setMat4(std::string name, glm::mat4 mat)
{
    
}