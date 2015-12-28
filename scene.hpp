#pragma once

#include <drawables.hpp>
#include <vector>

std::vector<dwg::Sphere> inline getSceneSpheres()
{
    std::vector<dwg::Sphere> spheres;
    dwg::Sphere s;

    // Light blue
    s.position = glm::vec3(5,0,18);
    s.radius = 5.0f;
    s.color =  glm::vec3(0.5,0.5,1);
    spheres.push_back(s);

    // purple
    s.position = glm::vec3(-5,-3,20);
    s.radius = 2.0f;
    s.color = glm::vec3(1,0,1);
    spheres.push_back(s);

    // blue
    s.position = glm::vec3(-10, 0,25.0f);
    s.radius = 5.0f;
    s.color = glm::vec3(0,0,1);
    spheres.push_back(s);

    // green
    s.position = glm::vec3(5,-4,4.0f);
    s.radius = 1.0f;
    s.color = glm::vec3(0,1,0);
    spheres.push_back(s);

    // Red
    s.position = glm::vec3(1,-3,5.0f);
    s.radius = 2.0f;
    s.color = glm::vec3(1,0,0);
    spheres.push_back(s);

    // Transparent
    s.position = glm::vec3(-5,-3,5.0f);
    s.radius = 2.0f;
    s.color = glm::vec3(1,1,1);
    spheres.push_back(s);

    // Yellow
    s.position = glm::vec3(-1,-3.3,1.0f);
    s.radius = 1.7f;
    s.color = glm::vec3(1,1,0);
    spheres.push_back(s);

    return spheres;
}

std::vector<dwg::Light> inline getSceneLights()
{
    std::vector<dwg::Light> lights;
    dwg::Light l;

    l.position = glm::vec3(0,15,20);
    l.color = glm::vec3(1,1,1);
    lights.push_back(l);

    l.position = glm::vec3(-10,15,40);
    l.color = glm::vec3(1,1,1);
    lights.push_back(l);

    l.position = glm::vec3(0,5,0);
    l.color = glm::vec3(2,2,2);
    lights.push_back(l);

    l.position = glm::vec3(0,10,0);
    l.color = glm::vec3(1,1,1);
    lights.push_back(l);

    return lights;
}

std::vector<dwg::Plane> inline getScenePlanes()
{
    std::vector<dwg::Plane> planes;
    dwg::Plane p;

    // Floor
    p.position = glm::vec3(0,-5,0);
    p.tileSize  = 2.5f;
    p.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    p.color = glm::vec3(1.0f, 1.0f, 1.0f);
    p.color2 = glm::vec3(0.0f, 0.0f, 0.0f);
    planes.push_back(p);
//    floor->type = Drawable::Type::REFLEXIVE;

    // Ceil
    p.position = glm::vec3(0,20,0);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(0, 1.0f, 0);
    p.color = glm::vec3(0.6f,0.6f,0.6f);
    planes.push_back(p);

    // Back
    p.position = glm::vec3(0,0,50);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(0,0,1);
    p.color = glm::vec3(.2f,0.2f,0.2f);
    planes.push_back(p);
//    back->specularColor = glm::vec3(2.0f,2.0f,2.0f);
//    back->type = Drawable::Type::REFLEXIVE;

    // Front
    p.position = glm::vec3(0,0,-10);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(0,0,-1);
    p.color = glm::vec3(.7f,0.7f,0.7f);
    planes.push_back(p);

    // Right wall
    p.position = glm::vec3(15,0,0);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(1,0,0);
    p.color = glm::vec3(1,0,0);

    planes.push_back(p);


    // Left wall
    p.position = glm::vec3(-15,0,0);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(-1,0,0);
    p.color = glm::vec3(0,0,1);
    planes.push_back(p);
    return planes;
}
