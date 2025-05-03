#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float ClampValue(float value, float min, float max);
float Vector3Angle(glm::vec3 a, glm::vec3 b);
glm::vec2 VecToSpheric(glm::vec3 vec);
glm::vec3 SphericToVec(glm::vec2 spheric);
glm::vec3 EclipticToEquitorial(glm::vec3 rotation, float tilt);
glm::vec3 EquitorialToEcliptic(glm::vec3 equitorial, float tilt);
glm::vec2 ConvRotAxisToNonAxisEcliptic(glm::vec3 rotation, glm::vec2 ecliptic);
