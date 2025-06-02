#include "sipManager.hpp"

SIPManager::SIPManager()
{
    speed = 1.0f;
    maxImages = 0;
    baseTime = -1;
    lastImage = 0;
    currentImageCount = 0;
    time = 0;
    lastDeleted = 0;
}

void SIPManager::Initalize(AppContext* context, int maxImages, float baseTime)
{
    images = new SIPImage[maxImages];
    deletedIndexes = new int[maxImages];
    this->baseTime = baseTime;
    this->maxImages = maxImages;

    pipeline.Initalize(ShaderSettings("SIPImage.vert", 0, 1, 0, 0), ShaderSettings("SIPImage.frag", 1, 0, 0, 0));
    pipeline.CreatePipeline(context, CreateDefaultVertAttributes(), sizeof(Vertex), 3);

    lastImage = 0;
    currentImageCount = 0;

    sipCamera.Initalize(context, 5);
}

void SIPManager::Update(AppContext* context, float deltaTime)
{
    time += deltaTime * speed;

    for(int i = 0; i < maxImages; i++)
    {
        if(!images[i].created)
          continue;

        images[i].ApplyRotation(earthRotationSpeed, earthOrbitSpeed, latitude, deltaTime * speed, time);
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

        images[i].DrawMesh(context, &pipeline, camera->GetProjMat(), camera->GetViewMat(), renderPass, cmbBuf);
    }
}


void SIPManager::LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context)
{
    //cap at max images
    if(lastImage >= maxImages)
        return;

    float deltaTime = 0;

    //get base time
    if(currentImageCount == 0 || baseTime == -1)
    {
        baseTime = time;
    }
    else if(time != baseTime && baseTime != -1)
    {
        //check for time passing
        float convTime = (floorf(this->time / 3600.0f) * 10000) + (floorf(this->time / 60.0f) * 100) + this->time;
        deltaTime = (convTime + baseTime) - time;
    }

    SIPImage image;
    image.CreateFromFile(file, azimuth, altitude, angularSize, deltaTime, context);
    //reset the time to what it should be
    image.time = time;

    //check if we can use a index out of the deleted
    if(lastDeleted > 0)
    {
        lastDeleted--;
        images[deletedIndexes[lastDeleted]] = image;
        currentImageCount++;
    }
    else
    {
        images[lastImage] = image;
        lastImage++;
        currentImageCount++;
    }
}

void SIPManager::LoadImageAbsolute(std::string location, float azimuth, float altitude, glm::vec2 angularSize, float time, AppContext* context)
{
    //cap at max images
    if(lastImage >= maxImages)
        return;

    float deltaTime = 0;

    //get base time
    if(currentImageCount == 0 || baseTime == -1)
    {
        baseTime = time;
    }
    else if(time != baseTime && baseTime != -1)
    {
        //check for time passing
        float convTime = (floorf(this->time / 3600.0f) * 10000) + (floorf(this->time / 60.0f) * 100) + this->time;
        deltaTime = (convTime + baseTime) - time;
    }

    SIPImage image;
    image.CreateFromLocation(location, azimuth, altitude, angularSize, deltaTime, context);
    //reset the time to what it should be
    image.time = time;

    //check if we can use a index out of the deleted
    if(lastDeleted > 0)
    {
        lastDeleted--;
        images[deletedIndexes[lastDeleted]] = image;
        currentImageCount++;
    }
    else
    {
        images[lastImage] = image;
        lastImage++;
        currentImageCount++;
    }
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
    else if(time != baseTime && baseTime != -1)
    {
        //check for time passing
        float convTime = (floorf(this->time / 3600.0f) * 10000) + (floorf(this->time / 60.0f) * 100) + this->time;
        deltaTime = (convTime + baseTime) - time;
    }

    SIPImage image;
    image.CreateFromSurface(surface, azimuth, altitude, angularSize, deltaTime, context);
    //reset the time to what it should be
    image.time = time;

    //check if we can use a index out of the deleted
    if(lastDeleted > 0)
    {
        lastDeleted--;
        images[deletedIndexes[lastDeleted]] = image;
        currentImageCount++;
    }
    else
    {
        images[lastImage] = image;
        lastImage++;
        currentImageCount++;
    }
}

void SIPManager::LoadImage(std::string file, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context)
{
    //cap at max images
    if(lastImage >= maxImages)
        return;

    float deltaTime = 0;

    //get base time
    if(currentImageCount == 0 || baseTime == -1)
    {
        baseTime = time;
    }
    else if(time != baseTime && baseTime != -1)
    {
        //check for time passing
        float convTime = (floorf(this->time / 3600.0f) * 10000) + (floorf(this->time / 60.0f) * 100) + this->time;
        deltaTime = (convTime + baseTime) - time;
    }

    SIPImage image;
    image.CreateFromFile(file, azimuth, altitude, angularSize, deltaTime, applyTilt, context);
    //reset the time to what it should be
    image.time = time;

    //check if we can use a index out of the deleted
    if(lastDeleted > 0)
    {
        lastDeleted--;
        images[deletedIndexes[lastDeleted]] = image;
        currentImageCount++;
    }
    else
    {
        images[lastImage] = image;
        lastImage++;
        currentImageCount++;
    }
}

void SIPManager::LoadImageAbsolute(std::string location, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context)
{
    //cap at max images
    if(lastImage >= maxImages)
        return;

    float deltaTime = 0;

    //get base time
    if(currentImageCount == 0 || baseTime == -1)
    {
        baseTime = time;
    }
    else if(time != baseTime && baseTime != -1)
    {
        //check for time passing
        float convTime = (floorf(this->time / 3600.0f) * 10000) + (floorf(this->time / 60.0f) * 100) + this->time;
        deltaTime = (convTime + baseTime) - time;
    }

    SIPImage image;
    image.CreateFromLocation(location, azimuth, altitude, angularSize, deltaTime, applyTilt, context);
    //reset the time to what it should be
    image.time = time;

    if(lastDeleted > 0)
    {
        lastDeleted--;
        images[deletedIndexes[lastDeleted]] = image;
        currentImageCount++;
    }
    else
    {
        images[lastImage] = image;
        lastImage++;
        currentImageCount++;
    }
}


void SIPManager::LoadImage(SDL_Surface* surface, float azimuth, float altitude, glm::vec2 angularSize, float time, bool applyTilt, AppContext* context)
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
    else if(time != baseTime && baseTime != -1)
    {
        //check for time passing
        float convTime = (floorf(this->time / 3600.0f) * 10000) + (floorf(this->time / 60.0f) * 100) + this->time;
        deltaTime = (convTime + baseTime) - time;
    }

    SIPImage image;
    image.CreateFromSurface(surface, azimuth, altitude, angularSize, deltaTime, applyTilt, context);
    //reset the time to what it should be
    image.time = time;

    if(lastDeleted > 0)
    {
        lastDeleted--;
        images[deletedIndexes[lastDeleted]] = image;
        currentImageCount++;
    }
    else
    {
        images[lastImage] = image;
        lastImage++;
        currentImageCount++;
    }
}


void SIPManager::DeleteImage(AppContext* context, int index)
{
    //check that we are not going to delete a empty image
    if(currentImageCount > 0)
    {
        images[index].Delete(context);
        currentImageCount--;
        deletedIndexes[lastDeleted] = index;
        lastDeleted++;
    }
}

void SIPManager::Clean(AppContext* context)
{
    for(int i = 0; i < maxImages; i++)
    {
        if(images[i].created)
            images[i].Delete(context);
    }

    pipeline.Delete(context);
    sipCamera.Clean(context);

    delete[] images;
    delete[] deletedIndexes;
}


float SIPManager::GetDayNightCycle()
{
    int component = ((int)(time / 3600.0f) % 24) / 6;
    float componentTime = fmod(time / 3600.0f, 6.0f);

    //the time is divided into 4 components
    if(component == 0)
    {
        //night
        return 0.0f;
    }
    else if(component == 1)
    {
        //night to day
        return componentTime / 6.0f;
    }
    else if(component == 2)
    {
        //day
        return 1.0f;
    }
    else if(component == 3)
    {
        //day to night
        return -componentTime / 6.0f + 1;
    }

    return 0.0f;
}
