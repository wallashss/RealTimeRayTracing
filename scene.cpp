
#include <drawables.hpp>
#include <vector>
#include <scene.h>

std::vector<dwg::Sphere> getDefaultSceneSpheres()
{
    std::vector<dwg::Sphere> spheres;
    dwg::Sphere s;

    // Light blue
    s.position = glm::vec3(5,0,18);
    s.radius = 5.0f;
    s.color =  glm::vec4(0.5,0.5,1, 1.2f);
    spheres.push_back(s);

    // purple
    s.position = glm::vec3(-5,-3,20);
    s.radius = 2.0f;
    s.color = glm::vec4(1,0,1, 0.0f);
    spheres.push_back(s);

    // blue
    s.position = glm::vec3(-10, 0,25.0f);
    s.radius = 5.0f;
    s.color = glm::vec4(0, 0, 1, 0.0f);
    spheres.push_back(s);

    // green
    s.position = glm::vec3(5,-4,4.0f);
    s.radius = 1.0f;
    s.color = glm::vec4(0, 1, 0, 0.0f);
    spheres.push_back(s);

    // Transparent
    s.position = glm::vec3(-5,-3,5.0f);
    s.radius = 2.0f;
    s.color = glm::vec4(1, 1, 1, -0.9f);
//    s.color = glm::vec4(1, 1, 1, 0.0f);
    spheres.push_back(s);

    // Yellow
    s.position = glm::vec3(-1, -3.3, 1.0f);
    s.radius = 1.7f;
    s.color = glm::vec4(1, 1, 0, -1.4f);
//    s.color = glm::vec4(1, 1, 0, 0.0f);
    spheres.push_back(s);

    // Red
    s.position = glm::vec3(1,-3, 5.0f);
    s.radius = 2.0f;
    s.color = glm::vec4(1, 0, 0, 0.0f);
    spheres.push_back(s);

    return spheres;
}


std::vector<dwg::Light> getDefaultSceneLights()
{
    std::vector<dwg::Light> lights;
    dwg::Light l;

    l.position = glm::vec3(0,5,0);
    l.color = glm::vec4(2,2,2,1.0f);
    lights.push_back(l);

    l.position = glm::vec3(0,15,20);
    l.color = glm::vec4(1,1,1,1.0f);
    lights.push_back(l);

//    l.position = glm::vec3(0,10,0);
//    l.color = glm::vec4(1,1,1,1.0f);
//    lights.push_back(l);
    return lights;
}

std::vector<dwg::Plane> getDefaultScenePlanes()
{
    std::vector<dwg::Plane> planes;
    dwg::Plane p;

    // Floor
    p.position = glm::vec3(0,-5,0);
    p.tileSize  = 2.5f;
    p.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    p.color1 = glm::vec4(1.0f, 1.0f, 1.0f, 1.2f);
    p.color2 = glm::vec4(0.0f, 0.0f, 0.0f, 1.2f);
    planes.push_back(p);

    // Right wall
    p.position = glm::vec3(15,0,0);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(1,0,0);
    p.color1 = glm::vec4(1,0,0,0.0f);
    planes.push_back(p);

    // Left wall
    p.position = glm::vec3(-15,0,0);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(-1,0,0);
    p.color1 = glm::vec4(0,0,1,0.0f);
    planes.push_back(p);

    // Back
    p.position = glm::vec3(0,0,50);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(0,0,1);
    p.color1 = glm::vec4(.2f,0.2f,0.2f, 1.2f);
    planes.push_back(p);

    // Front
    p.position = glm::vec3(0,0,-10);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(0,0,-1);
    p.color1 = glm::vec4(.7f,0.7f,0.7f, 1.2f);
    planes.push_back(p);

    // Ceil
    p.position = glm::vec3(0,20,0);
    p.tileSize  = 0.0f;
    p.normal = glm::vec3(0, 1.0f, 0);
    p.color1 = glm::vec4(0.6f,0.6f,0.6f, 0.0f);
    planes.push_back(p);


    return planes;
}
