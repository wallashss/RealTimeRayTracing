#pragma once

#include <glm/glm.hpp>

namespace dwg
{

struct Sphere
{
    glm::vec3 position;
    float radius;
    glm::vec4 color;
};

// All planes are chess for definition
struct Plane
{
    glm::vec3 position;
    float tileSize;
    glm::vec3 normal;
    float dummyFloat; // padding to align memory
    glm::vec4 color1;
    glm::vec4 color2;
};

struct Light
{
    glm::vec3 position;
    float dummyFloat; // padding to align memory
    glm::vec4 color;
};



}
