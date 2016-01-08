#include "raytracing.h"
#include <iostream>

#include <QFile>
#include <QTextStream>
#include <timer.h>


RayTracing::RayTracing(dwg::Scene scene, unsigned int glTexture, int textureWidth, int textureHeight) : _textureWidth(textureWidth), _textureHeight(textureHeight)
{
    localSizeX = 16;
    localSizeY = 16;

    // Create OpenCL context
    _clContext = std::make_shared<CLContextWrapper>();

    if(_clContext->createContextWithOpengl())
    {
        std::cout << "Successfully created OpenCL context with OpenGL" << std::endl;
        std::cout << "Max work group size " << _clContext->getMaxWorkGroupSize() << std::endl;
    }
    else
    {
        std::cout << "Failed to create context!" << std::endl;
        return;
    }

    if(!_clContext->hasCreatedContext())
    {
        return;
    }

    // Share glTexture
    _glTexture = glTexture;
    _sharedTextureBufferId = _clContext->shareGLTexture(_glTexture, BufferType::WRITE_ONLY);

    // Setup scene
    _numSpheres = static_cast<int>(scene.spheres.size());
    _numPlanes = static_cast<int>(scene.planes.size());
    _numLights = static_cast<int>(scene.lights.size());

    // Setup buffers
    _spheresBufferId = _clContext->createBufferFromArray(scene.spheres.size(), &(scene.spheres.data()[0]), BufferType::READ_ONLY);
    _planesBufferId  = _clContext->createBufferFromArray(scene.planes.size(),  &(scene.planes.data()[0]),  BufferType::READ_ONLY);
    _lightsBufferId  = _clContext->createBufferFromArray(scene.lights.size(),  &(scene.lights.data()[0]),  BufferType::READ_ONLY);

    // Temp Texture
    _tempColorsBufferId  = _clContext->createBuffer(  4*sizeof(float) * _textureWidth*_textureHeight, nullptr, BufferType::READ_AND_WRITE);

    // Prepare program
    QFile kernelSourceFile(":/cl_files/raytracing.cl");

    if(!kernelSourceFile.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        std::cout << "Failed to load cl file" << std::endl;
        return;
    }
    QTextStream kernelSourceTS(&kernelSourceFile);
    QString clSource = kernelSourceTS.readAll();

    _clContext->createProgramFromSource(clSource.toStdString());

    _clContext->prepareKernel("rayTracingKernel");
    _clContext->prepareKernel("drawToTextureKernel");
}

void testScan(CLContextWrapper * _clContext);

void RayTracing::update()
{
    if(!_clContext)
    {
        std::cout << "OpenCL context not intanced!" << std::endl;
        return;
    }
    if(!_clContext->hasCreatedContext())
    {
        std::cout << "OpenCL context not created!" << std::endl;
        return;
    }
    NDRange range;
    range.workDim = 2;
    range.globalOffset[0] = 0;
    range.globalOffset[1] = 0;
    range.globalSize[0] = _textureWidth;
    range.globalSize[1] = _textureHeight;
    range.localSize[0] = localSizeX;
    range.localSize[1] = localSizeY;

    int iterations = 6;

    size_t localTempSize = sizeof(float)*16*localSizeX*localSizeY;
    size_t localLightSize = sizeof(float)*8*_numLights;

    _clContext->dispatchKernel("rayTracingKernel", range, {&_tempColorsBufferId,
                                                    &_spheresBufferId, &_numSpheres,
                                                    &_planesBufferId, &_numPlanes,
                                                    &_lightsBufferId, &_numLights,
                                                    KernelArg::getShared(localTempSize),
                                                    KernelArg::getShared(localLightSize),
                                                    &iterations,
                                                    &_eye.x, &_eye.y, &_eye.z});


    _clContext->executeSafeAndSyncronized(&_sharedTextureBufferId, 1, [=] () mutable
    {
        _clContext->dispatchKernel("drawToTextureKernel", range, {&_sharedTextureBufferId,
                                                                  &_tempColorsBufferId});
    });
}

void RayTracing::setEye(glm::vec3 eye)
{
    _eye = eye;
}

glm::vec3 RayTracing::getEye() const
{
    return _eye;
}

