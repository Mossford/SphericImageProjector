#include "sipImage.hpp"

SIPImage::SIPImage()
{
    file = "";
    azimuth = 0;
    altitude = 0;
    angularSize = glm::vec2(0);
    time = 0;
    created = false;
    rotation = glm::vec3(0);
}

void SIPImage::CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, float earthRotation, AppContext* context)
{
    //for some reason this is multipled by pi
    angularSize *=  M_PI / 180.0f;

    this->file = file;
    this->azimuth = azimuth;
    this->altitude = altitude;
    this->angularSize = angularSize;
    this->time = time;

    image.LoadFromFile(context, file);
    mesh = Create2DQuadSpherical(glm::vec3(0), glm::vec3(azimuth, altitude, 1.0f), angularSize, 2);
    mesh.BufferGens(context);

    //calculate the starting rotation
    this->rotation.x = 23.4f - 45;
    this->rotation.y -= earthRotation * time;


    created = true;
}

void SIPImage::CreateFromSurface(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, float earthRotation, AppContext* context)
{
    //for some reason this is multipled by pi
    angularSize *=  M_PI / 180.0f;

    this->azimuth = azimuth;
    this->altitude = altitude;
    this->angularSize = angularSize;
    this->time = time;

    image.CreateFromSurface(context, surface);
    mesh = Create2DQuadSpherical(glm::vec3(0), glm::vec3(azimuth, altitude, 1.0f), angularSize, 2);
    mesh.BufferGens(context);

    //calculate the starting rotation
    //this->rotation.x = 23.4f;
    this->rotation.y -= earthRotation * time;


    created = true;
}


void SIPImage::UpdateMesh(AppContext* context)
{
    float degToRad = M_PI / 180.0f;

    glm::mat4 rotationMat = glm::mat4(1.0f);
    rotationMat = glm::rotate(rotationMat, rotation.x * degToRad, glm::vec3(1.0f,0.0f,0.0f));
    rotationMat = glm::rotate(rotationMat, rotation.y * degToRad, glm::vec3(0.0f,1.0f,0.0f));
    rotationMat = glm::rotate(rotationMat, rotation.z * degToRad, glm::vec3(0.0f,0.0f,1.0f));

    mesh.ProjectToSphere(rotationMat, glm::scale(glm::mat4(1.0f), glm::vec3(angularSize.x, angularSize.y, 1.0f)), 2);
    mesh.ReGenBuffer(context);
}

void SIPImage::DrawMesh(AppContext* context, glm::mat4 proj, glm::mat4 view, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf)
{
    mesh.modelMatrix = glm::mat4(1.0f);
    image.BindSampler(renderPass, 0);
    mesh.DrawMesh(context, renderPass, cmbBuf, proj, view);
}


void SIPImage::Delete(AppContext* context)
{
    image.Delete(context);
    created = false;
}

void SIPImage::ApplyRotation(float earthRotation, float latitude, float deltaTime)
{
    //since images are taken parallel to the ground we dont need to add in a x rotation
    //this->rotation.x = 23.4f - latitude;
    this->rotation.y -= earthRotation * deltaTime;
}

