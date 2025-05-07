#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 1) out vec3 aColorO;

layout(set = 1, binding = 0) uniform UniformBufferObject
{
    mat4 model;
} ubo;

void main()
{
    aColorO = aColor;
    gl_Position = ubo.model * vec4(aPos, 1.0);
}
