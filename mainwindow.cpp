#include "mainwindow.h"

#include <glview.h>
#include <drawables.hpp>
#include <scene.h>

#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

#include <iostream>

#include <timer.h>

#include <glm/gtx/rotate_vector.hpp>

static int textureWidth = 640;
static int textureHeight = 480;

static int localSizeX = 16;
static int localSizeY = 16;

static const glm::vec3 ORIGINAL_EYE(0,0,-40);



MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), _eye(ORIGINAL_EYE)
{
    setMinimumSize(1024, 768);

    QHBoxLayout * hlayout = new QHBoxLayout();

    _clContext = std::make_shared<CLContextWrapper>();

    // Spheres
    std::vector<dwg::Sphere> spheres = getSceneSpheres();
    _numSpheres = spheres.size();
    std::vector<float> fSpheres;

    for(size_t i =0 ; i < spheres.size(); i++)
    {
        const dwg::Sphere &sphere = spheres[i];

        fSpheres.push_back(sphere.position.x); // Position
        fSpheres.push_back(sphere.position.y);
        fSpheres.push_back(sphere.position.z);
        fSpheres.push_back(sphere.radius); // Radius

        fSpheres.push_back(sphere.color.r); // Color
        fSpheres.push_back(sphere.color.g);
        fSpheres.push_back(sphere.color.b);
        fSpheres.push_back(sphere.color.w);
    }

    // Planes
    std::vector<dwg::Plane> planes = getScenePlanes();
    _numPlanes = planes.size();
    std::vector<float> fPlanes;

    // We send 16 float to align memory
    for(size_t i =0 ; i < _numPlanes; i++)
    {
        const dwg::Plane &plane = planes[i];
        fPlanes.push_back(plane.position.x); // Pos
        fPlanes.push_back(plane.position.y);
        fPlanes.push_back(plane.position.z);
        fPlanes.push_back(plane.tileSize);
        fPlanes.push_back(plane.normal.x); // Normal
        fPlanes.push_back(plane.normal.y);
        fPlanes.push_back(plane.normal.z);
        fPlanes.push_back(0.0f);
        fPlanes.push_back(plane.color.r);  // Color
        fPlanes.push_back(plane.color.g);
        fPlanes.push_back(plane.color.b);
        fPlanes.push_back(plane.color.a);
        fPlanes.push_back(plane.color2.r); // Color 2
        fPlanes.push_back(plane.color2.g);
        fPlanes.push_back(plane.color2.b);
        fPlanes.push_back(plane.color.a);
    }


    // Lights

    auto lights = getSceneLights();
    _numLights = lights.size();
    std::vector<float> fLights;

    for(size_t i =0 ; i < lights.size(); i++)
    {
        const dwg::Light &light = lights[i];
        fLights.push_back(light.position.x);
        fLights.push_back(light.position.y);
        fLights.push_back(light.position.z);
        fLights.push_back(0.0f);

        fLights.push_back(light.color.r);
        fLights.push_back(light.color.g);
        fLights.push_back(light.color.b);
        fLights.push_back(1.0f);
    }

    _glView = new GLView([=] (GLView * newGlView) mutable
    {
        if(_clContext->createContextWithOpengl())
        {
            std::cout << "Successfully created OpenCL context with OpenGL" << std::endl;
        }

        _glTexture = newGlView->createTexture(textureWidth, textureHeight);
        _sharedTextureBufferId = _clContext->shareGLTexture(_glTexture, BufferType::WRITE_ONLY);

        _tempTextureBufferId = _clContext->createBufferWithBytes(sizeof(float)*4*textureWidth*textureHeight, nullptr, BufferType::READ_AND_WRITE);

        QFile kernelSourceFile(":/cl_files/raytracing.cl");

        if(!kernelSourceFile.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            std::cout << "Failed to load cl file" << std::endl;
            return;
        }
        QTextStream kernelSourceTS(&kernelSourceFile);
        QString clSource = kernelSourceTS.readAll();

        if(!_clContext->hasCreatedContext())
        {
            return;
        }

        _spheresBufferId = _clContext->createBuffer(fSpheres.size(), fSpheres.data(), BufferType::READ_ONLY);
        _planesBufferId  = _clContext->createBuffer(fPlanes.size(), fPlanes.data(), BufferType::READ_ONLY);
        _lightsBufferId  = _clContext->createBuffer(fLights.size(), fLights.data(), BufferType::READ_ONLY);
        _raysBufferId    = _clContext->createBufferWithBytes(16*16*4*2*sizeof(float), nullptr, BufferType::READ_AND_WRITE);
        _pixelsBufferId  = _clContext->createBufferWithBytes(16*16*2*sizeof(int), nullptr, BufferType::READ_AND_WRITE);

        _clContext->createProgramFromSource(clSource.toStdString());
        _clContext->prepareKernel("rayTracing");
        _clContext->prepareKernel("drawToTexture");
        _clContext->prepareKernel("prefixSum");

    });

    _qtimer = new QTimer(this);
    _qtimer->setInterval(16);
    QObject::connect(_qtimer, &QTimer::timeout, [&]
    {
        _eye = glm::rotateY(_eye, glm::pi<float>()*0.01f);
        _updateWithCL();
    });

    _glView->setFixedSize(textureWidth, textureHeight);

    _drawButton = new QPushButton("Draw");
    QObject::connect(_drawButton, &QPushButton::clicked,[=]
    {
        _eye = ORIGINAL_EYE;
        _updateWithCL();
    });

    QPushButton *rotateButton = new QPushButton("Rotate");
    QObject::connect(rotateButton, &QPushButton::clicked,[=]
    {
        if(!_qtimer->isActive())
        {
            _qtimer->start();
        }
        else
        {
            _qtimer->stop();
        }
    });

    hlayout->addWidget(_glView);
    hlayout->addWidget(_drawButton);
    hlayout->addWidget(rotateButton);

    setLayout(hlayout);
}



void MainWindow::testScan()
{
    int localSize = 512;
    int globalSize = 1 * localSize;
    int totalSize = 2 * globalSize;

//    typedef float VectorType;
    typedef int VectorType;
    std::vector<VectorType> input;

    NDRange range;
    range.workDim = 1;
    range.globalOffset[0] = 0;
    range.globalSize[0] = globalSize;
    range.localSize[0] = localSize;

    for (int i = 0 ; i < totalSize ; i++)
    {
        input.push_back(1);
    }

    auto bufferInput  = _clContext->createBuffer(input.size(), input.data(), BufferType::READ_AND_WRITE);
    auto bufferOutput = _clContext->createBufferWithBytes(totalSize*sizeof(int), nullptr, BufferType::READ_AND_WRITE);


    util::Timer t;
    _clContext->dispatchKernel("prefixSum", range, {&bufferInput,
                                                    &bufferOutput,
                                                    KernelArg::getShared(sizeof(VectorType) * 2 * localSize),
                                                    &localSize});
    _clContext->finish();

    static std::vector<float> times;
    if(times.size() <= 100)
    {
        times.push_back(t.elapsedMilliSec());
    }

//    std::cout << t.elapsedMilliSec() << std::endl;

    std::vector<VectorType> output;
    output.resize(totalSize);
    _clContext->dowloadArrayFromBuffer(bufferOutput, totalSize, output.data());

    bool right = true;
    VectorType sum = 0;
    for(int i =0 ; i < output.size(); i++)
    {
//        std::cout << i << " " << sum << " " << output[i] ;
        if(output[i] != sum)
        {
            right = false;
//            std::cout << " << wrong ";
        }
//        std::cout << std::endl;
        sum += input[i];
    }

//    std::cout << "banks" << std::endl;
//    for(int i =0 ; i < output.size(); i++)
//    {
//        std::cout << i +1 << " " << output[i] << " " << output[i] - (output[i]/16) * 16 << std::endl;
//    }


    if(!right)
    {
//        std::cout << "Wrong!" << std::endl;
    }
    if(times.size() == 100)
    {
        float allTime = 0;
        for(int i = 10 ; i < 100; i++ )
        {
            allTime += times[i];
        }
        times.clear();
        std::cout << "Avg " << (allTime/90) << std::endl;
    }

}

void MainWindow::_updateWithCL()
{
    util::Timer t;


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

    testScan();

    _clContext->dispatchKernel("rayTracing", range, {&_tempTextureBufferId,
                                                    &_spheresBufferId, &_numSpheres,
                                                    &_planesBufferId, &_numPlanes,
                                                    &_lightsBufferId, &_numLights,
                                                    KernelArg::getShared(localTempSize),
                                                    KernelArg::getShared(localLightSize),
                                                    &_eye.x, &_eye.y, &_eye.z});


    _glView->makeCurrent();
    _glView->setBaseTexture(_glTexture);
    _clContext->executeSafeAndSyncronized(&_sharedTextureBufferId, 1, [=] () mutable
    {
        _clContext->dispatchKernel("drawToTexture", range, {&_sharedTextureBufferId,
                                                        &_tempTextureBufferId});
//        clContext->dispatchKernel("rayTracing", range, {&_sharedTextureBufferId,
//                                                        &_spheresBufferId, &_numSpheres,
//                                                        &_planesBufferId, &_numPlanes,
//                                                        &_lightsBufferId, &_numLights,
//                                                        KernelArg::getShared(localTempSize),
//                                                        KernelArg::getShared(localLightSize),
//                                                        &_eye.x, &_eye.y, &_eye.z});

    });

    _glView->doneCurrent();
    _glView->repaint();
    float elapsedTime = t.elapsedMilliSec();
    setWindowTitle(QString::fromStdString("Rendered: ") + QString::fromStdString(std::to_string(elapsedTime)) + QString(" ms") +
                   QString(" (") + QString::fromStdString(std::to_string((1.0f/elapsedTime)*1e3f)) + QString(" FPS)"));
//    std::cout << "Raytracing time: "<< t.elapsedMilliSec();
    t.restart();


//    std::cout << " Rendering time: "<< elapsedTime << std::endl;

}


MainWindow::~MainWindow()
{

}
