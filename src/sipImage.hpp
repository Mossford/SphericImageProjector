#pragma once
#include <glm/glm.hpp>
#include <string>
#include "texture.hpp"

struct AppContext;

class SIPImage
{
public:

    std::string file;
    float azimuth;
    float altitude;
    glm::vec2 angularSize;
    //time the image was taken
    float time;

    Texture image;
    Mesh mesh;

    SIPImage();
    void CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context);
    void Delete(AppContext* context);
};
