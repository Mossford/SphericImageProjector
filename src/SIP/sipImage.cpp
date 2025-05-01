#include "sipImage.hpp"
#include "app.hpp"

SIPImage::SIPImage()
{
    file = "";
    azimuth = 0;
    altitude = 0;
    angularSize = glm::vec2(0);
    time = 0;
    created = false;
    applyTilt = true;
    rotation = glm::vec3(0);
}

void SIPImage::CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context)
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
    if(applyTilt)
        this->rotation.x = context->sipManager.latitude;
    this->rotation.y -= context->sipManager.earthRotationSpeed * time;


    created = true;
}

void SIPImage::CreateFromSurface(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context)
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
    if(applyTilt)
        this->rotation.x = context->sipManager.latitude;
    this->rotation.y -= context->sipManager.earthRotationSpeed * time;


    created = true;
}

void SIPImage::CreateFromFile(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context)
{
    //for some reason this is multipled by pi
    angularSize *=  M_PI / 180.0f;

    this->file = file;
    this->azimuth = azimuth;
    this->altitude = altitude;
    this->angularSize = angularSize;
    this->time = time;
    this->applyTilt = applyTilt;

    image.LoadFromFile(context, file);
    mesh = Create2DQuadSpherical(glm::vec3(0), glm::vec3(azimuth, altitude, 1.0f), angularSize, 2);
    mesh.BufferGens(context);

    //calculate the starting rotation
    if(applyTilt)
        this->rotation.x = context->sipManager.latitude;
    this->rotation.y -= context->sipManager.earthRotationSpeed * time;


    created = true;
}

void SIPImage::CreateFromSurface(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context)
{
    //for some reason this is multipled by pi
    angularSize *=  M_PI / 180.0f;

    this->azimuth = azimuth;
    this->altitude = altitude;
    this->angularSize = angularSize;
    this->time = time;
    this->applyTilt = applyTilt;

    image.CreateFromSurface(context, surface);
    mesh = Create2DQuadSpherical(glm::vec3(0), glm::vec3(azimuth, altitude, 1.0f), angularSize, 2);
    mesh.BufferGens(context);

    //calculate the starting rotation
    if(applyTilt)
        this->rotation.x = context->sipManager.latitude;
    this->rotation.y -= context->sipManager.earthRotationSpeed * time;


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

void SIPImage::DrawMesh(AppContext* context, Pipeline* pipeline, glm::mat4 proj, glm::mat4 view, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf)
{
    mesh.modelMatrix = glm::mat4(1.0f);
    image.BindSampler(renderPass, 0);
    pipeline->vertexShader.AddMat4(proj * view * mesh.modelMatrix);
    pipeline->vertexShader.BindVertexUniformData(cmbBuf, 0);
    mesh.DrawMesh(context, renderPass, cmbBuf);
}


void SIPImage::Delete(AppContext* context)
{
    image.Delete(context);
    created = false;
}

void SIPImage::ApplyRotation(float earthRotation, float earthOrbit, float latitude, float deltaTime, float time)
{
    if(applyTilt)
    {
        this->rotation.x = latitude;
    }
    this->rotation.y -= (earthOrbit + earthRotation) * deltaTime;
}

