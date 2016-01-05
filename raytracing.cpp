#include "raytracing.h"
#include <iostream>

#include <QFile>
#include <QTextStream>
#include <timer.h>

//static int localSizeX = 16;
//static int localSizeY = 16;

RayTracing::RayTracing(dwg::Scene scene, unsigned int glTexture, int textureWidth, int textureHeight) : _textureWidth(textureWidth), _textureHeight(textureHeight)
{
    localSizeX = 16;
    localSizeY = 16;
    _clContext = std::make_shared<CLContextWrapper>();

    if(_clContext->createContextWithOpengl())
    {
        std::cout << "Successfully created OpenCL context with OpenGL" << std::endl;
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
    _numSpheres = scene.spheres.size();
    _numPlanes = scene.planes.size();
    _numLights = scene.lights.size();

    // Setup buffers
    _spheresBufferId = _clContext->createBufferFromArray(scene.spheres.size(), &(scene.spheres.data()[0]), BufferType::READ_ONLY);
    _planesBufferId  = _clContext->createBufferFromArray(scene.planes.size(),  &(scene.planes.data()[0]),  BufferType::READ_ONLY);
    _lightsBufferId  = _clContext->createBufferFromArray(scene.lights.size(),  &(scene.lights.data()[0]),  BufferType::READ_ONLY);

    _raysBufferId       = _clContext->createBuffer(4*2*sizeof(float) * _textureWidth*_textureHeight, nullptr, BufferType::READ_AND_WRITE);
    _pixelsBufferId     = _clContext->createBuffer(  2*sizeof(int)   * _textureWidth*_textureHeight, nullptr, BufferType::READ_AND_WRITE);
    _tempColorsBufferId = _clContext->createBuffer(  4*sizeof(float) * _textureWidth*_textureHeight, nullptr, BufferType::READ_AND_WRITE);

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
    _clContext->prepareKernel("primaryRayTracingKernel");
    _clContext->prepareKernel("drawToTextureKernel");
    _clContext->prepareKernel("prefixSumKernel");

}


void RayTracing::update()
{
    NDRange range;
    range.workDim = 2;
    range.globalOffset[0] = 0;
    range.globalOffset[1] = 0;
    range.globalSize[0] = 640;
    range.globalSize[1] = 480;
    range.localSize[0] = localSizeX;
    range.localSize[1] = localSizeY;

    size_t localTempSize = sizeof(float)*16*localSizeX*localSizeY;
    size_t localLightSize = sizeof(float)*8*_numLights;

    _clContext->dispatchKernel("primaryRayTracingKernel", range, {&_tempColorsBufferId,
                                                    &_spheresBufferId, &_numSpheres,
                                                    &_planesBufferId, &_numPlanes,
                                                    &_lightsBufferId, &_numLights,
                                                    KernelArg::getShared(localTempSize),
                                                    KernelArg::getShared(localLightSize),
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


//void testScan()
//{
//    bool PRINT_RESULT = false;
//    int localSize = 256;
//    int globalSize = 1 * localSize;
//    int totalSize = 2 * globalSize;

////    typedef float VectorType;
//    typedef int VectorType;
//    std::vector<VectorType> input;

//    NDRange range;
//    range.workDim = 1;
//    range.globalOffset[0] = 0;
//    range.globalSize[0] = globalSize;
//    range.localSize[0] = localSize;

//    for (int i = 0 ; i < totalSize ; i++)
//    {
//        input.push_back(1);
//    }

//    auto bufferInput  = _clContext->createBuffer(input.size(), input.data(), BufferType::READ_ONLY);
//    auto bufferOutput = _clContext->createBufferWithBytes(totalSize*sizeof(int), nullptr, BufferType::READ_AND_WRITE);


//    util::Timer t;
//    _clContext->dispatchKernel("prefixSumKernel", range, {&bufferInput,
//                                                          &bufferOutput,
//                                                          KernelArg::getShared(sizeof(VectorType) * 2 * localSize),
//                                                          &localSize});
//    _clContext->finish();

//    static std::vector<float> times;
//    if(times.size() <= 100)
//    {
//        times.push_back(t.elapsedMilliSec());
//    }

////    std::cout << t.elapsedMilliSec() << std::endl;

//    std::vector<VectorType> output;
//    output.resize(totalSize);
//    _clContext->dowloadArrayFromBuffer(bufferOutput, totalSize, output.data());

//    bool right = true;
//    VectorType sum = 0;
//    for(size_t i =0 ; i < output.size(); i++)
//    {
//        if(PRINT_RESULT)
//        {
//            std::cout << i << " " << sum << " " << output[i] ;
//        }

//        if(output[i] != sum)
////        if(output[i] != i)
//        {
//            right = false;
//            if(PRINT_RESULT)
//            {
//                std::cout << " << wrong ";
//            }
//        }
//        if(PRINT_RESULT)
//        {
//            std::cout << std::endl;
//        }
//        sum += input[i];
//    }

////    std::cout << "banks" << std::endl;
////    for(int i =0 ; i < output.size(); i++)
////    {
////        std::cout << i +1 << " " << output[i] << " " << output[i] - (output[i]/16) * 16 << std::endl;
////    }


//    if(!right && PRINT_RESULT)
//    {
//        std::cout << "Wrong!" << std::endl;
//    }
//    if(times.size() == 100)
//    {
//        float allTime = 0;
//        for(int i = 10 ; i < 100; i++ )
//        {
//            allTime += times[i];
//        }
//        times.clear();
//        std::cout << "Avg " << (allTime/90) << std::endl;
//    }
//}
