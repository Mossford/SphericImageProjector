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

glm::vec2 VecToSpheric(glm::vec3 vec)
{
    glm::vec2 rot;
    rot.x = atan2(vec.x, -vec.z);
    rot.y = asin(vec.y);

    float radToDeg = 180.0f / M_PI;

    rot.x *= radToDeg;
    //check if the x is negative
    if(rot.x < 0.0f)
        rot.x += 360.0f;
    rot.y *= radToDeg;

    return rot;
}

glm::vec3 SphericToVec(glm::vec2 spheric)
{
    float degToRad = M_PI / 180.0f;

    //https://en.wikipedia.org/wiki/Spherical_coordinate_system
    glm::vec3 target;
    target.x = sin(spheric.x * degToRad) * cos(spheric.y * degToRad);
    target.y = sin(spheric.y * degToRad);
    target.z = -cos(spheric.x * degToRad) * cos(spheric.y * degToRad);

    return target;
}

glm::vec3 EclipticToEquitorial(glm::vec3 ecliptic, float tilt)
{
    //https://en.wikipedia.org/wiki/Ecliptic_coordinate_system
    float degToRad = M_PI / 180.0f;
    tilt *= degToRad;

    glm::vec3 equ;
    equ.x = ecliptic.x;
    equ.y = cos(tilt) * ecliptic.y - (sin(tilt) * ecliptic.z);
    equ.z = sin(tilt) * ecliptic.y + (cos(tilt) * ecliptic.z);

    float radToDeg = 1.0f / degToRad;

    equ.y *= radToDeg;
    equ.z *= radToDeg;

    return equ;
}

glm::vec3 EquitorialToEcliptic(glm::vec3 equitorial, float tilt)
{
    //https://en.wikipedia.org/wiki/Ecliptic_coordinate_system
    float degToRad = M_PI / 180.0f;
    tilt *= degToRad;

    glm::vec3 equ;
    equ.x = equitorial.x;
    equ.y = cos(tilt) * equitorial.y + (sin(tilt) * equitorial.z);
    equ.z = -sin(tilt) * equitorial.y + (cos(tilt) * equitorial.z);

    float radToDeg = 1.0f / degToRad;

    equ.y *= radToDeg;
    equ.z *= radToDeg;

    return equ;
}

glm::vec2 ConvRotAxisToNonAxisEcliptic(glm::vec3 rotation, glm::vec2 ecliptic)
{
    float degToRad = M_PI / 180.0f;

    //create a inverse rotation matrix to rotate the image to the desired rotation
    glm::mat4 rotationMat = glm::mat4(1.0f);
    rotationMat = glm::rotate(rotationMat, rotation.x * degToRad, glm::vec3(1.0f,0.0f,0.0f));
    rotationMat = glm::rotate(rotationMat, rotation.y * degToRad, glm::vec3(0.0f,1.0f,0.0f));
    rotationMat = glm::rotate(rotationMat, rotation.z * degToRad, glm::vec3(0.0f,0.0f,1.0f));
    rotationMat = glm::inverse(rotationMat);

    //treat the inputed ecliptic as if it was already rotated by the matrix and rotate it back to the "desired" location
    glm::vec3 pos = SphericToVec(glm::vec2(ecliptic.x, ecliptic.y));
    pos = glm::vec3(rotationMat * glm::vec4(pos, 1.0f));

    return VecToSpheric(pos);
}
