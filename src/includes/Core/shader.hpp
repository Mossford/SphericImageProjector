#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <typeinfo>
#include <cstring>
#include <string>
#include <vector>
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
private:

    //the combined data to be passed as a uniform
    std::vector<Uint8> buffer;

public:

    SDL_GPUShader* shader;
    Shader();
    void CompileShader(std::string location, SDL_GPUDevice* device, ShaderSettings settings, bool parse);
    void AddBool(bool value);
    void AddInt(int value);
    void AddFloat(float value);
    void AddVec2(glm::vec2 value);
    void AddVec3(glm::vec3 value);
    void AddVec4(glm::vec4 value);
    void AddMat4(glm::mat4 mat);
    void BindVertexUniformData(SDL_GPUCommandBuffer* cmbBuf, int slot);
    void BindFragmentUniformData(SDL_GPUCommandBuffer* cmbBuf, int slot);
    
};

SDL_GPUShader* CompileShaderProgram(std::string location, std::string file, SDL_GPUDevice* device, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount, bool parse);
