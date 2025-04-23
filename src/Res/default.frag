#version 460 core
layout(location = 0) out vec4 out_color;

//uniform sampler2D diffuseTexture;

void main()
{           
    //vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb
    
    out_color = vec4(vec3(1.0), 1.0);
}