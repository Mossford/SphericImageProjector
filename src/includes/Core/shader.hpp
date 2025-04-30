#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <glm/glm.hpp>
#include <iostream>
#include <typeinfo>
#include <cstring>
#include <string>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

struct ShaderSettings
{
    std::string file;
    Uint32 samplerCount;
    Uint32 uniformBufferCount;
    Uint32 storageBufferCount;
    Uint32 storageTextureCount;

    ShaderSettings() {}
    ShaderSettings(std::string file, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount)
    {
        this->file = file;
        this->samplerCount = samplerCount;
        this->uniformBufferCount = uniformBufferCount;
        this->storageBufferCount = storageBufferCount;
        this->storageTextureCount = storageTextureCount;
    }
};

class Shader
{
public:

    SDL_GPUShader* shader;
    Shader();
    void CompileShader(std::string location, SDL_GPUDevice* device, ShaderSettings settings, bool parse);
    void setBool(std::string name, bool value);
    void setInt(std::string name, int value);
    void setFloat(std::string name, float value);
    void setVec2(std::string name, glm::vec2 value);
    void setVec2(std::string name, float x, float y);
    void setVec3(std::string name, glm::vec3 value);
    void setVec3(std::string name, float x, float y, float z);
    void setVec4(std::string name, glm::vec4 value);
    void setVec4(std::string name, float x, float y, float z, float w);
    void setMat4(std::string name, glm::mat4 mat);
    
};

SDL_GPUShader* CompileShaderProgram(std::string location, std::string file, SDL_GPUDevice* device, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount, bool parse);
