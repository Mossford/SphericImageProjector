#include "sipManager.hpp"

SIPManager::SIPManager()
{

}

void SIPManager::Initalize(AppContext* context, int maxImages, float baseTime)
{
    images = new SIPImage[maxImages];
    this->baseTime = baseTime;
    this->maxImages = maxImages;

    pipeline.Initalize(ShaderSettings("defaultSIPImage.vert", 0, 1, 0, 0), ShaderSettings("defaultSIPImage.frag", 1, 0, 0, 0));
    pipeline.CreatePipeline(context);

    lastImage = 0;
    currentImageCount = 0;
}

void SIPManager::Update(AppContext* context, float deltaTime)
{
    for(int i = 0; i < maxImages; i++)
    {
        if(!images[i].created)
          continue;

        images[i].ApplyRotation(earthRotationSpeed, latitude, deltaTime);
        images[i].UpdateMesh(context);
    }
}

void SIPManager::Draw(AppContext* context, Camera* camera, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf)
{
    pipeline.Bind(renderPass);

    glm::mat4 proj = camera->GetProjMat();

    for(int i = 0; i < maxImages; i++)
    {
        if(!images[i].created)
            continue;

        images[i].DrawMesh(context, camera->GetProjMat(), camera->GetViewMat(), renderPass, cmbBuf);
    }
}


void SIPManager::LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context)
{
    //cap at max images
    if(lastImage >= maxImages)
        return;

    float deltaTime = 0;

    //get base time
    if(currentImageCount == 0)
    {
        baseTime = time;
    }
    else if(time != baseTime)
    {
        //check for time passing
        deltaTime = baseTime - time;
    }

    SIPImage image;
    image.CreateFromFile(file, azimuth, altitude, angularSize, deltaTime, earthRotationSpeed, context);
    images[lastImage] = image;
    lastImage++;
    currentImageCount++;
}

void SIPManager::LoadImage(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context)
{
    //cap at max images
    if(lastImage >= maxImages)
        return;

    float deltaTime = 0;

    //get base time and reset the base time if the base time is -1
    if(currentImageCount == 0 || baseTime == -1)
    {
        baseTime = time;
    }
    else if(time != baseTime)
    {
        //check for time passing
        deltaTime = baseTime - time;
    }

    SIPImage image;
    image.CreateFromSurface(surface, azimuth, altitude, angularSize, deltaTime, earthRotationSpeed, context);
    images[lastImage] = image;
    lastImage++;
    currentImageCount++;
}


void SIPManager::DeleteImage(AppContext* context)
{
    //check that we are not going to delete a empty image
    if(currentImageCount > 0)
    {
        images[lastImage].Delete(context);
        lastImage--;
        currentImageCount--;
    }
}

void SIPManager::Clean(AppContext* context)
{
    for(int i = 0; i < maxImages; i++)
    {
        if(images[i].created)
            images[i].Delete(context);
    }

    delete[] images;
}



