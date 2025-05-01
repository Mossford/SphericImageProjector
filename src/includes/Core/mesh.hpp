#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "vertex.hpp"
#include "math.hpp"

struct AppContext;

class Mesh
{
private:
SDL_GPUBuffer* vertexBuffer;
SDL_GPUBuffer* indexBuffer;

public:

    std::vector<Vertex> vertexes;
    std::vector<unsigned int> indices;
    glm::vec3 position, rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    glm::mat4 rotMatrix;

    Mesh();
    Mesh(const Mesh &other);
    Mesh(std::vector<Vertex> vertexes, std::vector<unsigned int> indices, glm::vec3 position, glm::vec3 rotation, float scale);
    void BufferGens(AppContext* context);
    void ReGenBuffer(AppContext* context);
    void DrawMesh(AppContext* context, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmbBuf);
    void Delete(AppContext* context);
    void CreateModelMat();
    void CreateRotationMat();
    void FixWindingOrder();
    void CreateSmoothNormals();
    void SubdivideTriangle();
    void Balloon(float delta, float speed, float percentage);
    void ProjectToSphere(glm::mat4 rotationMat, glm::mat4 scaleMat, int subdivideNum);
};

Mesh Create2DTriangle(glm::vec3 position, glm::vec3 rotation);
Mesh Create2DQuad(glm::vec3 position, glm::vec3 rotation);
Mesh Create2DQuadSpherical(glm::vec3 position, glm::vec3 rotation, glm::vec2 scale, unsigned int subdivideNum);
Mesh CreateCubeMesh(glm::vec3 position, glm::vec3 rotation);
Mesh CreateSphereMesh(glm::vec3 position, glm::vec3 rotation, unsigned int subdivideNum);
Mesh CreateCubeSphereMesh(glm::vec3 position, glm::vec3 rotation, unsigned int subdivideNum);
Mesh CreateUvSquare(glm::vec3 position, glm::vec3 rotation, unsigned int subdivideNum);
