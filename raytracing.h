#pragma once

#include <clcontextwrapper.h>
#include <scene.h>

#include <memory>

class RayTracing
{
public:
    RayTracing(dwg::Scene scene, unsigned int glTexture, int textureWidth, int textureHeight);

    void update();

    void setEye(glm::vec3 eye);

    glm::vec3 getEye() const;

private:

    void _compactRays(BufferId rays, int count);

    void _prefixSum(BufferId input, BufferId output, int n);

    void testScan();

private:

    // Shared OpenGL texture
    unsigned int _glTexture;
    BufferId _sharedTextureBufferId;

    // Spheres
    BufferId _spheresBufferId;
    int _numSpheres;

    // Planes
    BufferId _planesBufferId;
    int _numPlanes;

    // Lights
    BufferId _lightsBufferId;
    int _numLights;

    // Temp buffer
    BufferId _tempColorsBufferId;

    glm::vec3 _eye;

    int _textureWidth;
    int _textureHeight;

    size_t localSizeX;
    size_t localSizeY;

    std::shared_ptr<CLContextWrapper> _clContext;

};

