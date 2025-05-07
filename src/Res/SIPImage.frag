#version 460 core
layout(location = 0) out vec4 out_color;

layout (location = 2) in vec2 aUV;

layout(set = 2, binding = 0) uniform sampler2D textureSamp;

void main()
{          
    vec3 color = vec3(1.0);
    color = texture(textureSamp, aUV).rgb;
    
    out_color = vec4(color, 1.0);
}
