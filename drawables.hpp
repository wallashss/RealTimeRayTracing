#pragma once

#include <glm/glm.hpp>

namespace dwg
{
struct Drawables
{
    glm::vec3 color;
    glm::vec3 position;
};


struct Sphere : Drawables
{
    glm::vec3 position;
    float radius;
};

struct Plane : Drawables
{
    glm::vec3 pointA;
    glm::vec3 pointB;
};

struct Light : Drawables
{

};



}
