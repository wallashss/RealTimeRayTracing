#pragma once

#include <drawables.hpp>
#include <vector>

namespace dwg
{
    typedef struct
    {
        std::vector<dwg::Sphere> spheres;
        std::vector<dwg::Plane> planes;
        std::vector<dwg::Light> lights;
    } Scene;
}

std::vector<dwg::Sphere> getDefaultSceneSpheres();

std::vector<dwg::Light> getDefaultSceneLights();

std::vector<dwg::Plane> getDefaultScenePlanes();
