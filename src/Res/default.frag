#version 460 core
layout(location = 0) out vec4 out_color;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

//uniform sampler2D diffuseTexture;

void main()
{           
    //vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb
    
    out_color = vec4(aNormal, 1.0);
}