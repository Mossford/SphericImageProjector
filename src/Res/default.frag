#version 460 core
layout(location = 0) out vec4 out_color;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

layout(set = 2, binding = 0) uniform sampler2D textureSamp;

#define PI 3.14159265358979323846

float DegToUvX(float deg)
{
    return deg / 360.0;
}

//for nothern hemisphere, 90 degrees needs to be added
float DegToUvY(float deg)
{
    return deg / 180.0;
}

void main()
{          
    //for some reason the uv coord calculation only works in the shader
    vec2 uv = vec2(0.5 + (atan(aPos.z, aPos.x) / (2.0 * PI)), 0.5 + (asin(aPos.y) / PI));

    vec3 color = vec3(1.0);
    for(int y = 90; y < 180; y += 5)
    {
        float imgX = 270;
        float imgY = y;
        if(uv.x >= DegToUvX(imgX) && uv.x <= DegToUvX(imgX + 3.41) && uv.y >= DegToUvY(imgY) && uv.y <= DegToUvY(imgY + 2.28))
        {
            //scale to 0-1 for the small section
            vec2 uvScale = vec2((uv.x - DegToUvX(imgX)) / (DegToUvX(imgX + 3.41) - DegToUvX(imgX)), (uv.y - DegToUvY(imgY)) / (DegToUvY(imgY + 2.28) - DegToUvY(imgY)));
            color = texture(textureSamp, uvScale).rgb;
        }
    }

    //color *= aNormal;
    
    out_color = vec4(color, 1.0);
}
