#include "shader.hpp"

struct SpirVBinary
{
    uint32_t *words; // SPIR-V words
    int size; // number of words in SPIR-V binary
};

SpirVBinary compileShaderToSPIRV_Vulkan(glslang_stage_t stage, const char* shaderSource, const char* fileName);

Shader::Shader()
{

}

SDL_GPUShader* CompileShaderProgram(std::string location, std::string file, SDL_GPUDevice* device, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount)
{
    SDL_GPUShaderStage stage;
	if (SDL_strstr(file.c_str(), ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(file.c_str(), ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		SDL_Log("Invalid shader stage!");
		SDL_Log(SDL_GetError());
        return NULL;
	}

	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
	//for hlsl this will only be main
	const char *entrypoint;
	format = SDL_GPU_SHADERFORMAT_SPIRV;
	entrypoint = "main";

	size_t codeSize;
	void* code = SDL_LoadFile((location + file).c_str(), &codeSize);
	if (code == NULL)
	{
		SDL_Log("Failed to load shader from disk! %s", (location + file).c_str());
		SDL_Log(SDL_GetError());
		return NULL;
	}

	/*SpirVBinary bin;
	if(stage == SDL_GPU_SHADERSTAGE_VERTEX)
		bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_VERTEX, (char*)code, (location + file).c_str());
	else if (stage == SDL_GPU_SHADERSTAGE_FRAGMENT)
		bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_FRAGMENT, (char*)code, (location + file).c_str());

	codeSize = bin.size;
	code = bin.words;*/

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
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL)
	{
		SDL_Log("Failed to create shader!");
		SDL_Log(SDL_GetError());
		SDL_free(code);
		return NULL;
	}


	SDL_free(code);

	return shader;
}

void Shader::CompileShader(std::string location, std::string file, SDL_GPUDevice* device, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount)
{
    SDL_GPUShaderStage stage;
	if (SDL_strstr(file.c_str(), ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(file.c_str(), ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		SDL_Log("Invalid shader stage!");
		SDL_Log(SDL_GetError());
        return;
	}

	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
	//for hlsl this will only be main
	const char *entrypoint;
	format = SDL_GPU_SHADERFORMAT_SPIRV;
	entrypoint = "main";

	size_t codeSize;
	void* code = SDL_LoadFile((location + file).c_str(), &codeSize);
	if (code == NULL)
	{
		SDL_Log("Failed to load shader from disk! %s", (location + file).c_str());
		SDL_Log(SDL_GetError());
		return;
	}

	SpirVBinary bin;
	if(stage == SDL_GPU_SHADERSTAGE_VERTEX)
		bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_VERTEX, (char*)code, (location + file).c_str());
	else if (stage == SDL_GPU_SHADERSTAGE_FRAGMENT)
		bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_FRAGMENT, (char*)code, (location + file).c_str());

	codeSize = bin.size;
	code = bin.words;

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
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL)
	{
		SDL_Log("Failed to create shader!");
		SDL_Log(SDL_GetError());
		SDL_free(code);
		return;
	}

	this->shader = shader;

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

SpirVBinary compileShaderToSPIRV_Vulkan(glslang_stage_t stage, const char* shaderSource, const char* fileName)
{
    const glslang_input_t input = 
	{
        .language = GLSLANG_SOURCE_GLSL,
        .stage = stage,
        .client = GLSLANG_CLIENT_VULKAN,
        .client_version = GLSLANG_TARGET_VULKAN_1_2,
        .target_language = GLSLANG_TARGET_SPV,
        .target_language_version = GLSLANG_TARGET_SPV_1_5,
        .code = shaderSource,
        .default_version = 460,
        .default_profile = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = true,
        .forward_compatible = false,
        .messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = glslang_default_resource(),
    };

    glslang_shader_t* shader = glslang_shader_create(&input);

    SpirVBinary bin = 
	{
        .words = NULL,
        .size = 0,
    };

    if (!glslang_shader_preprocess(shader, &input))
	{
        SDL_Log("GLSL preprocessing failed %s\n", fileName);
        SDL_Log("%s\n", glslang_shader_get_info_log(shader));
        SDL_Log("%s\n", glslang_shader_get_info_debug_log(shader));
        SDL_Log("%s\n", input.code);
        glslang_shader_delete(shader);
        return bin;
    }

    if (!glslang_shader_parse(shader, &input))
	{
        SDL_Log("GLSL parsing failed %s\n", fileName);
        SDL_Log("%s\n", glslang_shader_get_info_log(shader));
        SDL_Log("%s\n", glslang_shader_get_info_debug_log(shader));
        SDL_Log("%s\n", glslang_shader_get_preprocessed_code(shader));
        glslang_shader_delete(shader);
        return bin;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
	{
        SDL_Log("GLSL linking failed %s\n", fileName);
        SDL_Log("%s\n", glslang_program_get_info_log(program));
        SDL_Log("%s\n", glslang_program_get_info_debug_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        return bin;
    }

    glslang_program_SPIRV_generate(program, stage);

    bin.size = glslang_program_SPIRV_get_size(program);
    bin.words = (uint32_t*)malloc(bin.size * sizeof(uint32_t));
    glslang_program_SPIRV_get(program, bin.words);

    const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages)
        SDL_Log("(%s) %s\b", fileName, spirv_messages);

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return bin;
}