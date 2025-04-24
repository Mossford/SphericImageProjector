#include "mesh.hpp"

Mesh::Mesh()
{

}

Mesh::Mesh(const Mesh &other)
{
    this->vertexes = other.vertexes;
    this->indices = other.indices;
    this->position = other.position;
    this->rotation = other.rotation;
    this->scale = other.scale;
}

Mesh::Mesh(std::vector<Vertex> vertexes, std::vector<unsigned int> indices, glm::vec3 position, glm::vec3 rotation, float scale)
{
    this->vertexes = vertexes;
    this->indices = indices;
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;
}

void Mesh::Delete(AppContext* context)
{
    SDL_ReleaseGPUBuffer(context->gpuDevice, vertexBuffer);
    SDL_ReleaseGPUBuffer(context->gpuDevice, indexBuffer);
}

void Mesh::BufferGens(AppContext* context)
{
    // Create the vertex buffer
	SDL_GPUBufferCreateInfo bufferCreateInfoVert = {};
	bufferCreateInfoVert.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	bufferCreateInfoVert.size = sizeof(Vertex) * vertexes.size();

    SDL_GPUBufferCreateInfo bufferCreateInfoInd = {};
	bufferCreateInfoInd.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	bufferCreateInfoInd.size = sizeof(Uint32) * indices.size();

	vertexBuffer = SDL_CreateGPUBuffer(context->gpuDevice, &bufferCreateInfoVert);
    indexBuffer = SDL_CreateGPUBuffer(context->gpuDevice, &bufferCreateInfoInd);

	SDL_GPUTransferBufferCreateInfo bufferTransferInfo = {};
	bufferTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	bufferTransferInfo.size = sizeof(Vertex) * vertexes.size();

	// To get data into the vertex buffer, we have to use a transfer buffer
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(context->gpuDevice, &bufferTransferInfo);

	void* transferData = SDL_MapGPUTransferBuffer(context->gpuDevice, transferBuffer, false);

    Vertex* vertexData = (Vertex*)transferData;
    for (int i = 0; i < vertexes.size(); i++)
    {
        vertexData[i] = vertexes[i];
    }

    Uint32* indexData = (Uint32*) &vertexData[vertexes.size()];
    for (int i = 0; i < indices.size(); i++)
    {
        indexData[i] = indices[i];
    }
    
	SDL_UnmapGPUTransferBuffer(context->gpuDevice, transferBuffer);

	// Upload the transfer data to the vertex buffer
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->gpuDevice);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_GPUTransferBufferLocation bufferlocVert = {};
	bufferlocVert.transfer_buffer = transferBuffer;
	bufferlocVert.offset = 0;

	SDL_GPUBufferRegion bufferregionVert = {};
	bufferregionVert.buffer = vertexBuffer;
	bufferregionVert.offset = 0;
	bufferregionVert.size = sizeof(Vertex) * vertexes.size();

    SDL_GPUTransferBufferLocation bufferlocInd = {};
    bufferlocInd.transfer_buffer = transferBuffer;
    bufferlocInd.offset = sizeof(Vertex) * vertexes.size();

    SDL_GPUBufferRegion bufferregionInd = {};
    bufferregionInd.buffer = indexBuffer;
    bufferregionInd.offset = 0;
    bufferregionInd.size = sizeof(Uint32) * indices.size();

	SDL_UploadToGPUBuffer(copyPass, &bufferlocVert, &bufferregionVert, false);
    SDL_UploadToGPUBuffer(copyPass, &bufferlocInd, &bufferregionInd, false);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(context->gpuDevice, transferBuffer);
}

void Mesh::ReGenBuffer()
{
    
    
}

void Mesh::DrawMesh(AppContext* context, SDL_GPURenderPass* renderPass)
{
    SDL_GPUBufferBinding bufBindVert = {};
	bufBindVert.buffer = vertexBuffer;
	bufBindVert.offset = 0;
	SDL_BindGPUVertexBuffers(renderPass, 0, &bufBindVert, 1);
    SDL_GPUBufferBinding bufBindInd = {};
	bufBindInd.buffer = indexBuffer;
	bufBindInd.offset = 0;
    SDL_BindGPUIndexBuffer(renderPass, &bufBindInd, SDL_GPU_INDEXELEMENTSIZE_32BIT);
    //SDL_DrawGPUPrimitives(renderPass, vertexes.size(), 1, 0, 0);
	SDL_DrawGPUIndexedPrimitives(renderPass, indices.size(), 1, 0, 0, 0);
}

void Mesh::CreateModelMat()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x * 3.14159265358979323846f/180.0f, glm::vec3(1.0f,0.0f,0.0f));
    model = glm::rotate(model, rotation.y * 3.14159265358979323846f/180.0f, glm::vec3(0.0f,1.0f,0.0f));
    model = glm::rotate(model, rotation.z * 3.14159265358979323846f/180.0f, glm::vec3(0.0f,0.0f,1.0f));
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    modelMatrix = model;
}

void Mesh::CreateRotationMat()
{
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::rotate(mat, rotation.x * 3.14159265358979323846f/180.0f, glm::vec3(1.0f,0.0f,0.0f));
    mat = glm::rotate(mat, rotation.y * 3.14159265358979323846f/180.0f, glm::vec3(0.0f,1.0f,0.0f));
    mat = glm::rotate(mat, rotation.z * 3.14159265358979323846f/180.0f, glm::vec3(0.0f,0.0f,1.0f));
    rotMatrix = mat;
}

void Mesh::FixWindingOrder()
{
    if(vertexes.size() != 0)
    {
        glm::vec3 vertex1, vertex2, vertex3;
        for (unsigned int i = 0; i < vertexes.size() / 3; i++)
        {
            vertex1 = vertexes[i * 3].position; vertex2 = vertexes[i * 3 + 1].position; vertex1 = vertexes[i * 3 + 2].position;
            glm::vec3 u = vertex2 - vertex1;
            glm::vec3 v = vertex3 - vertex1;

            glm::vec3 normal = glm::cross(u,v);
            if(normal.z < 0)
            {
                unsigned int ind1, ind2, ind3;
                ind1 = indices[i * 3];
                ind2 = indices[i * 3 + 1];
                ind3 = indices[i * 3 + 2];
                indices[i * 3] = ind3;
                indices[i * 3 + 1] = ind2;
                indices[i * 3 + 2] = ind1;
            }
        }
        
    }
}

//---------------------------
//Needs fixing
//---------------------------
void Mesh::CreateSmoothNormals()
{
    for (unsigned int v = 0; v < vertexes.size(); v++)
    {
        glm::vec3 normal;
        for (unsigned int i = 0; i < indices.size(); i += 3)
        {
            unsigned int a,b,c;
            a = indices[i];
            b = indices[i + 1];
            c = indices[i + 2];
            if(vertexes[v].position == vertexes[a].position)
            {
                glm::vec3 u = vertexes[b].position - vertexes[a].position;
                glm::vec3 v = vertexes[c].position - vertexes[a].position;
                glm::vec3 tmpnormal = glm::normalize(glm::cross(u,v));
                normal += tmpnormal;
            }
            if(vertexes[v].position == vertexes[b].position)
            {
                glm::vec3 u = vertexes[a].position - vertexes[b].position;
                glm::vec3 v = vertexes[c].position - vertexes[b].position;
                glm::vec3 tmpnormal = glm::normalize(glm::cross(u,v));
                normal += tmpnormal;
            }
            if(vertexes[v].position == vertexes[c].position)
            {
                glm::vec3 u = vertexes[b].position - vertexes[c].position;
                glm::vec3 v = vertexes[a].position - vertexes[c].position;
                glm::vec3 tmpnormal = glm::normalize(glm::cross(u,v));
                normal += tmpnormal;
            }
        }
        vertexes[v].normal = glm::normalize(normal);
    }
}
 
void Mesh::SubdivideTriangle()
{
    std::vector<unsigned int> newIndices;
    std::vector<Vertex> newVerts;
    for (unsigned int i = 0; i < indices.size(); i += 3)
    {
        //ia is the row in the vertices array
        unsigned int ia = indices[i]; 
        unsigned int ib = indices[i + 1];
        unsigned int ic = indices[i + 2]; 
        Vertex a = vertexes[ia];
        Vertex b = vertexes[ib];
        Vertex c = vertexes[ic];
        glm::vec3 ab = (a.position + b.position) * 0.5f;
        glm::vec3 bc = (b.position + c.position) * 0.5f;
        glm::vec3 ca = (c.position + a.position) * 0.5f;
        glm::vec3 u = bc - ab;
        glm::vec3 v = ca - ab;
        glm::vec3 normal = glm::normalize(glm::cross(u,v));
        ia = newVerts.size();
        newVerts.push_back(a);
        ib = newVerts.size();
        newVerts.push_back(b);
        ic = newVerts.size();
        newVerts.push_back(c);
        unsigned int iab = newVerts.size();
        newVerts.push_back(Vertex(ab, normal, glm::vec2(0)));
        unsigned int ibc = newVerts.size(); 
        newVerts.push_back(Vertex(bc, normal, glm::vec2(0)));
        unsigned int ica = newVerts.size(); 
        newVerts.push_back(Vertex(ca, normal, glm::vec2(0)));
        newIndices.push_back(ia); newIndices.push_back(iab); newIndices.push_back(ica);
        newIndices.push_back(ib); newIndices.push_back(ibc); newIndices.push_back(iab);
        newIndices.push_back(ic); newIndices.push_back(ica); newIndices.push_back(ibc);
        newIndices.push_back(iab); newIndices.push_back(ibc); newIndices.push_back(ica);
    }
    indices.clear();
    indices = newIndices;
    vertexes.clear();
    vertexes = newVerts;
}

//
//
//
//
//
//
//
//
//--------------------Mesh Generation--------------------------

Mesh Create2DTriangle(glm::vec3 position, glm::vec3 rotation)
{
    std::vector<float> verts =
    {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
    };
    std::vector<float> normals =
    {
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        0.0f,1.0f,0.0f
    };
    std::vector<float> uv =
    {
        0.0f,1.0f,
        0.0f,1.0f,
        0.0f,1.0f
    };
    std::vector<Vertex> vertxes;
    Vertex tmpvertex = Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    vertxes.push_back(tmpvertex);
    tmpvertex.position = glm::vec3(1.0f, -1.0f, 0.0f);
    vertxes.push_back(tmpvertex);
    tmpvertex.position = glm::vec3(-1.0f, 1.0f, 0.0f);
    vertxes.push_back(tmpvertex);
    
    std::vector<unsigned int> indices =
    {
        0, 1, 2
    };
    return Mesh(vertxes, indices, position, rotation, 1);
}

Mesh CreateCubeMesh(glm::vec3 position, glm::vec3 rotation)
{
    std::vector<Vertex> vertexes = {
        Vertex(glm::vec3(-1.0f, -1.0f, 1.0f),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(-1.0f, 1.0f, 1.0f),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(-1.0f, -1.0f, -1.0f),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(-1.0f, 1.0f, -1.0f),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3( 1.0f,-1.0f, 1.0f),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(1.0f,1.0f, 1.0f),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(1.0f,-1.0f, -1.0f),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(1.0f,1.0f, -1.0f),glm::vec3(0), glm::vec2(0))
    };
    std::vector<unsigned int> indices =
    {
        1, 2, 0,
        3, 6, 2,
        7, 4, 6,
        5, 0, 4,
        6, 0, 2,
        3, 5, 7,
        1, 3, 2,
        3, 7, 6,
        7, 5, 4,
        5, 1, 0,
        6, 4, 0,
        3, 1, 5
    };
    Mesh mesh = Mesh(vertexes, indices, position, rotation, 1.0f);
    return mesh;
}

Mesh CreateSphereMesh(glm::vec3 position, glm::vec3 rotation, unsigned int subdivideNum)
{

    float t = 0.52573111f;  
    float b = 0.850650808f;

    std::vector<Vertex> vertexes = {
        Vertex(glm::vec3(-t,  b,  0),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(t,  b,  0),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(-t, -b,  0),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(t, -b,  0),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(0, -t,  b),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(0,  t,  b),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(0, -t, -b),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(0,  t, -b),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(b,  0, -t),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(b,  0,  t),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(-b,  0, -t),glm::vec3(0), glm::vec2(0)),
        Vertex(glm::vec3(-b,  0,  t),glm::vec3(0), glm::vec2(0))
    };

    std::vector<unsigned int> indices = {
        0, 11, 5, 
        0, 5, 1,
        0, 1, 7,
        0, 7, 10,
        0, 10, 11,
        
        1, 5, 9,
        5, 11, 4,
        11, 10, 2,
        10, 7, 6,
        7, 1, 8,
        
        3, 9, 4,
        3, 4, 2,
        3, 2, 6,
        3, 6, 8,
        3, 8, 9,
        
        4, 9, 5,
        2, 4, 11,
        6, 2, 10,
        8, 6, 7,
        9, 8, 1
    };

    std::vector<float> normals;
    std::vector<float> uv;

    for (unsigned int i = 0; i < subdivideNum; i++)
    {
        std::vector<unsigned int> newIndices;
        std::vector<Vertex> newVerts;
        for (unsigned int i = 0; i < indices.size(); i += 3)
        {
            //Get the required vertexes
            unsigned int ia = indices[i]; 
            unsigned int ib = indices[i + 1];
            unsigned int ic = indices[i + 2]; 
            Vertex a = vertexes[ia];
            Vertex b = vertexes[ib];
            Vertex c = vertexes[ic];

            //Create New Points
            glm::vec3 ab = glm::normalize((a.position + b.position) * 0.5f);
            glm::vec3 bc = glm::normalize((b.position + c.position) * 0.5f);
            glm::vec3 ca = glm::normalize((c.position + a.position) * 0.5f);

            //Create Normals
            glm::vec3 u = bc - ab;
            glm::vec3 v = ca - ab;
            glm::vec3 normal = glm::normalize(glm::cross(u,v));

            //Add the new vertexes
            ia = newVerts.size();
            newVerts.push_back(a);
            ib = newVerts.size();
            newVerts.push_back(b);
            ic = newVerts.size();
            newVerts.push_back(c);
            unsigned int iab = newVerts.size();
            newVerts.push_back(Vertex(ab, normal, glm::vec2(0)));
            unsigned int ibc = newVerts.size(); 
            newVerts.push_back(Vertex(bc, normal, glm::vec2(0)));
            unsigned int ica = newVerts.size(); 
            newVerts.push_back(Vertex(ca, normal, glm::vec2(0)));
            newIndices.push_back(ia); newIndices.push_back(iab); newIndices.push_back(ica);
            newIndices.push_back(ib); newIndices.push_back(ibc); newIndices.push_back(iab);
            newIndices.push_back(ic); newIndices.push_back(ica); newIndices.push_back(ibc);
            newIndices.push_back(iab); newIndices.push_back(ibc); newIndices.push_back(ica);
        }
        indices = newIndices;
        vertexes = newVerts;
    }
    Mesh mesh = Mesh(vertexes, indices, position, rotation, 1.0f);
    return mesh;
}