#pragma once

#include <glm/glm.hpp>

namespace dwg
{
struct Drawables
{
    glm::vec4 color;
    glm::vec3 position;
};


struct Sphere : Drawables
{
    glm::vec3 position;
    float radius;
};

// All planes are chess for definition
struct Plane : Drawables
{
    float tileSize;
    glm::vec3 normal;
    glm::vec4 color2;
};

struct Light : Drawables
{

};



}
