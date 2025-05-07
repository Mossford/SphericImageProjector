#version 460 core
layout(location = 0) out vec4 out_color;

layout (location = 2) in vec2 aUV;

layout(set = 2, binding = 0) uniform sampler2D accumFrame;
layout(set = 2, binding = 1) uniform sampler2D renderFrame;

layout(set = 3, binding = 0) uniform UniformBufferObject
{
    float mixAmount;
} ubo;

void main()
{
    vec3 renderColor = texture(renderFrame, aUV).rgb;
    vec3 accumColor = texture(accumFrame, aUV).rgb;

    vec3 color = mix(accumColor, renderColor, ubo.mixAmount);

    out_color = vec4(color, 1.0);
}
