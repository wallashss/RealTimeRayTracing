#pragma once

#include <glm/glm.hpp>

namespace dwg
{
struct Drawables
{
    glm::vec3 color;
};


struct Sphere : Drawables
{
    glm::vec3 position;
    float radius;
};



}
