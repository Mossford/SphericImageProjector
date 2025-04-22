#version 460 core
out vec4 out_color;

in VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

//uniform sampler2D diffuseTexture;

void main()
{           
    //vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    
    out_color = vec4(normal, 1.0);
}