#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "vertex.hpp"

class Mesh
{
private:

    unsigned int vao, vbo, ebo;

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
    void BufferGens();
    void ReGenBuffer();
    void DrawMesh();
    void Delete();
    void CreateModelMat();
    void CreateRotationMat();
    void FixWindingOrder();
    void CreateSmoothNormals();
    void SubdivideTriangle();
};

Mesh CreateSphereMesh(glm::vec3 position, glm::vec3 rotation, unsigned int subdivideNum);