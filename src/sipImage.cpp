#include "sipImage.hpp"

SIPImage::SIPImage()
{

}

void SIPImage::CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context)
{
    this->file = file;
    this->azimuth = azimuth;
    this->altitude = altitude;
    this->angularSize = angularSize;
    this->time = time;

    image.LoadFromFile(context, file);
}

void SIPImage::Delete(AppContext* context)
{
    image.Delete(context);
}
