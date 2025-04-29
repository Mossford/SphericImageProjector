#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

layout (location = 0) out vec3 aPosO;
layout (location = 1) out vec3 aNormalO;
layout (location = 2) out vec2 aUVO;

layout(set = 1, binding = 0) uniform UniformBufferObject
{
    mat4 model;
} ubo;

void main()
{
    aPosO = aPos;
    aNormalO = aNormal;
    aUVO = aUV;
    gl_Position = ubo.model * vec4(aPos, 1.0);
}
