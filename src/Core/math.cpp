#include "math.hpp"

float ClampValue(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

float Vector3Angle(glm::vec3 a, glm::vec3 b)
{
    float denominator = sqrt(glm::dot(a, a) * glm::dot(b, b));
    if (denominator < 1e-15f)
        return 0.0f;
    float dot = ClampValue(glm::dot(a, b) / denominator, -1.0f, 1.0f);
    return (acos(dot)) * 180.0f / M_PI;
}
