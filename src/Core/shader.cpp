#include "shader.hpp"

struct SpirVBinary
{
    Uint32 *words; // SPIR-V words
    int size; // number of words in SPIR-V binary
};

SpirVBinary compileShaderToSPIRV_Vulkan(glslang_stage_t stage, const char* shaderSource, const char* fileName);

Shader::Shader()
{

}

SDL_GPUShader* CompileShaderProgram(std::string location, std::string file, SDL_GPUDevice* device, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount, bool parse)
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
		SDL_Log("%s", SDL_GetError());
        return NULL;
	}

	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
	//for glsl this will only be main
	const char *entrypoint;
	format = SDL_GPU_SHADERFORMAT_SPIRV;
	entrypoint = "main";

	size_t codeSize;
	void* code = SDL_LoadFile((location + file).c_str(), &codeSize);
	if (code == NULL)
	{
		SDL_Log("Failed to load shader from disk! %s", (location + file).c_str());
		SDL_Log("%s", SDL_GetError());
		return NULL;
	}

	SpirVBinary bin;
	if(parse)
	{
		if(stage == SDL_GPU_SHADERSTAGE_VERTEX)
			bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_VERTEX, (char*)code, (location + file).c_str());
		else if (stage == SDL_GPU_SHADERSTAGE_FRAGMENT)
			bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_FRAGMENT, (char*)code, (location + file).c_str());

		//since glslang returns as uint32 we need to change it to uint8
		codeSize = bin.size * sizeof(Uint32) / sizeof(Uint8);
		code = bin.words;
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
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL)
	{
		SDL_Log("Failed to create shader!");
		SDL_Log("%s", SDL_GetError());
		SDL_free(code);
		return NULL;
	}


	SDL_free(code);

	return shader;
}

void Shader::CompileShader(std::string location, SDL_GPUDevice* device, ShaderSettings settings, bool parse)
{
    SDL_GPUShaderStage stage;
	if (SDL_strstr(settings.file.c_str(), ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(settings.file.c_str(), ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		SDL_Log("Invalid shader stage!");
		SDL_Log("%s", SDL_GetError());
        return;
	}

	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
	//for glsl this will only be main
	const char *entrypoint;
	format = SDL_GPU_SHADERFORMAT_SPIRV;
	entrypoint = "main";

	size_t codeSize;
	void* code = SDL_LoadFile((location + settings.file).c_str(), &codeSize);
	if (code == NULL)
	{
		SDL_Log("Failed to load shader from disk! %s", (location + settings.file).c_str());
		SDL_Log("%s", SDL_GetError());
		return;
	}

	SpirVBinary bin;
	if(parse)
	{
		if(stage == SDL_GPU_SHADERSTAGE_VERTEX)
			bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_VERTEX, (char*)code, (location + settings.file).c_str());
		else if (stage == SDL_GPU_SHADERSTAGE_FRAGMENT)
			bin = compileShaderToSPIRV_Vulkan(GLSLANG_STAGE_FRAGMENT, (char*)code, (location + settings.file).c_str());

		//since glslang returns as uint32 we need to change it to uint8
		codeSize = bin.size * sizeof(Uint32) / sizeof(Uint8);
		code = bin.words;
	}

	SDL_GPUShaderCreateInfo shaderInfo =
	{
		.code_size = codeSize,
		.code = (Uint8*)code,
		.entrypoint = entrypoint,
		.format = format,
		.stage = stage,
		.num_samplers = settings.samplerCount,
		.num_storage_textures = settings.storageTextureCount,
		.num_storage_buffers = settings.storageBufferCount,
		.num_uniform_buffers = settings.uniformBufferCount,
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL)
	{
		SDL_Log("Failed to create shader!");
		SDL_Log("%s", SDL_GetError());
		SDL_free(code);
		return;
	}

	this->shader = shader;

	SDL_free(code);
}   

void Shader::AddBool(bool value)
{
	Uint8* data = (Uint8*)&value;
	buffer.insert(buffer.end(), data, data + sizeof(bool));
}

void Shader::AddInt(int value)
{
	Uint8* data = (Uint8*)&value;
	buffer.insert(buffer.end(), data, data + sizeof(int));
}

void Shader::AddFloat(float value)
{
	Uint8* data = (Uint8*)&value;
	buffer.insert(buffer.end(), data, data + sizeof(float));
}

void Shader::AddVec2(glm::vec2 value)
{
	Uint8* data = (Uint8*)glm::value_ptr(value);
	buffer.insert(buffer.end(), data, data + sizeof(glm::vec2));
}

void Shader::AddVec3(glm::vec3 value)
{
	Uint8* data = (Uint8*)glm::value_ptr(value);
	buffer.insert(buffer.end(), data, data + sizeof(glm::vec3));
}

void Shader::AddVec4(glm::vec4 value)
{
	Uint8* data = (Uint8*)glm::value_ptr(value);
	buffer.insert(buffer.end(), data, data + sizeof(glm::vec4));
}

void Shader::AddMat4(glm::mat4 mat)
{
	Uint8* data = (Uint8*)glm::value_ptr(mat);
	buffer.insert(buffer.end(), data, data + sizeof(glm::mat4));
}

void Shader::BindVertexUniformData(SDL_GPUCommandBuffer* cmbBuf, int slot)
{
	SDL_PushGPUVertexUniformData(cmbBuf, slot, buffer.data(), buffer.size());
	buffer.clear();
}

void Shader::BindFragmentUniformData(SDL_GPUCommandBuffer* cmbBuf, int slot)
{
	SDL_PushGPUFragmentUniformData(cmbBuf, slot, buffer.data(), buffer.size());
	buffer.clear();
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
        .default_profile = GLSLANG_CORE_PROFILE,
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
	bin.words = new Uint32[bin.size];
    glslang_program_SPIRV_get(program, bin.words);

    const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages)
        SDL_Log("(%s) %s\b", fileName, spirv_messages);

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return bin;
}
