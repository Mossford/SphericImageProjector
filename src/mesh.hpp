#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "vertex.hpp"
#include "app.hpp"
#include "math.hpp"

#include <SDL3/SDL.h>

class Mesh
{
private:
SDL_GPUBuffer* vertexBuffer;
SDL_GPUBuffer* indexBuffer;

public:

    std::vector<Vertex> vertexes;
    std::vector<unsigned int> indices;
    glm::vec3 position, rotation;
    float scale;
    glm::mat4 modelMatrix;
    glm::mat4 rotMatrix;

    Mesh();
    Mesh(const Mesh &other);
    Mesh(std::vector<Vertex> vertexes, std::vector<unsigned int> indices, glm::vec3 position, glm::vec3 rotation, float scale);
    void BufferGens(AppContext* context);
    void ReGenBuffer();
    void DrawMesh(AppContext* context, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf, glm::mat4 proj, glm::mat4 view);
    void Delete(AppContext* context);
    void CreateModelMat();
    void CreateRotationMat();
    void FixWindingOrder();
    void CreateSmoothNormals();
    void SubdivideTriangle();
};

Mesh Create2DTriangle(glm::vec3 position, glm::vec3 rotation);
Mesh CreateCubeMesh(glm::vec3 position, glm::vec3 rotation);
Mesh CreateSphereMesh(glm::vec3 position, glm::vec3 rotation, unsigned int subdivideNum);