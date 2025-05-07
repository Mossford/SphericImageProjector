#version 460 core
layout(location = 0) out vec4 out_color;

layout (location = 1) in vec3 aColor;

void main()
{
    out_color = vec4(aColor, 1.0);
}

