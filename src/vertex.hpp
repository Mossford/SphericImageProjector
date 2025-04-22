#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv) 
    {
        this->position = position;
        this->normal = normal;
        this->uv = uv;
    }
    Vertex(const Vertex &other)
    {
        this->position = other.position;
        this->normal = other.normal;
        this->uv = other.uv;
    }
    ~Vertex() {}
};