#include "mainwindow.h"

#include <glview.h>
#include <drawables.hpp>
#include <scene.hpp>

#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

#include <iostream>

#include <timer.h>

#include <glm/gtx/rotate_vector.hpp>

static int textureWidth = 640;
static int textureHeight = 480;
static const glm::vec3 ORIGINAL_EYE(0,0,-30);


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), _eye(ORIGINAL_EYE)
{
    setMinimumSize(1024, 768);

    QHBoxLayout * hlayout = new QHBoxLayout();

    clContext = std::make_shared<CLContextWrapper>();

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
        fSpheres.push_back(1.0f);
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
        fPlanes.push_back(1.0f);
        fPlanes.push_back(plane.color2.r); // Color 2
        fPlanes.push_back(plane.color2.g);
        fPlanes.push_back(plane.color2.b);
        fPlanes.push_back(1.0f);
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
        if(clContext->createContextWithOpengl())
        {
            std::cout << "Successfully created OpenCL context with OpenGL" << std::endl;
        }

        _glTexture = newGlView->createTexture(textureWidth, textureHeight);
        _sharedTextureBufferId = clContext->shareGLTexture(_glTexture, BufferType::WRITE_ONLY);

        QFile kernelSourceFile(":/cl_files/raytracing.cl");

        if(!kernelSourceFile.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            std::cout << "Failed to load cl file" << std::endl;
            return;
        }
        QTextStream vertexTextStream(&kernelSourceFile);
        QString clSource = vertexTextStream.readAll();

        if(!clContext->hasCreatedContext())
        {
            return;
        }

        _spheresBufferId = clContext->createBuffer(fSpheres.size(), fSpheres.data(), BufferType::READ_ONLY);
        _planesBufferId = clContext->createBuffer(fPlanes.size(), fPlanes.data(), BufferType::READ_ONLY);
        _lightsBufferId = clContext->createBuffer(fLights.size(), fLights.data(), BufferType::READ_ONLY);


        clContext->createProgramFromSource(clSource.toStdString());
        clContext->prepareKernel("rayTracing");


    });

    _qtimer = new QTimer(this);
    _qtimer->setInterval(16);
    QObject::connect(_qtimer, &QTimer::timeout, [&]
    {
        _eye = glm::rotateY(_eye, glm::pi<float>()*0.01f);
        std::cout << "Eye " << _eye.x << " " << _eye.y << " " << _eye.z << std::endl;
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

void MainWindow::_updateWithCL()
{
    util::Timer t;
    _glView->makeCurrent();
    _glView->setBaseTexture(_glTexture);

    NDRange range;
    range.workDim = 2;
    range.globalOffset[0] = 0;
    range.globalOffset[1] = 0;
    range.globalSize[0] = 640;
    range.globalSize[1] = 480;
    range.localSize[0] = 16;
    range.localSize[1] = 12;

    clContext->executeSafeAndSyncronized(&_sharedTextureBufferId, 1, [=] () mutable
    {

        size_t localTempSize = sizeof(float)*16*12*16;
        size_t localLightSize = sizeof(float)*8*_numLights;

        clContext->dispatchKernel("rayTracing", range, {&_sharedTextureBufferId,
                                                        &_spheresBufferId, &_numSpheres,
                                                        &_planesBufferId, &_numPlanes,
                                                        &_lightsBufferId, &_numLights,
                                                        KernelArg::getShared(localTempSize),
                                                        KernelArg::getShared(localLightSize),
                                                        &_eye.x, &_eye.y, &_eye.z,
                                                        &textureWidth, &textureHeight});
    });

    _glView->doneCurrent();

    _glView->repaint();
    float elapsedTime = t.elapsedMilliSec();
    setWindowTitle(QString::fromStdString("Rendered: ") + QString::fromStdString(std::to_string(elapsedTime)) + QString(" ms") +
                   QString(" (") + QString::fromStdString(std::to_string((1.0f/elapsedTime)*1e3f)) + QString(" FPS)"));
    std::cout << "Raytracing time: "<< t.elapsedMilliSec();
    t.restart();


    std::cout << " Rendering time: "<< elapsedTime << std::endl;

}


MainWindow::~MainWindow()
{

}
