#include "mainwindow.h"

#include <glview.h>
#include <drawables.hpp>

#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

#include <iostream>

#include <timer.h>

#include <glm/gtx/rotate_vector.hpp>

static int textureWidth = 640;
static int textureHeight = 480;


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
    s.position = glm::vec3(-10,0,25.0f);
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
    s.position = glm::vec3(-5,-3.5,5.0f);
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

    p.position = glm::vec3(0,-5,0);
    p.pointA = glm::vec3(1,-5,0);
    p.pointB = glm::vec3(1,-5,1);
    p.color = glm::vec3(1.0f, 1.0f, 1.0f);
    planes.push_back(p);

//    floor->color1 = Drawable::blackColor;
//    floor->color2 = glm::vec3(1.5f,1.5f,1.5f);
//    floor->tileSize = 2.5f;

//    floor->type = Drawable::Type::REFLEXIVE;

//    Plane * ceil = new Plane("ceil");

    p.position = glm::vec3(0,20,0);
    p.pointA = glm::vec3(1,20,1);
    p.pointB = glm::vec3(1,20,0);
    p.color = glm::vec3(0.6f,0.6f,0.6f);
    planes.push_back(p);


//    Plane * back = new Plane("back");

    p.position = glm::vec3(0,0,50);
    p.pointA = glm::vec3(1,0,50);
    p.pointB = glm::vec3(1,1,50);
    p.color = glm::vec3(.2f,0.2f,0.2f);
    planes.push_back(p);
//    back->specularColor = glm::vec3(2.0f,2.0f,2.0f);
//    back->type = Drawable::Type::REFLEXIVE;

//    Plane * front = new Plane("front");

    p.position = glm::vec3(0,0,-10);
    p.pointA = glm::vec3(1,1,-10);
    p.pointB = glm::vec3(1,0,-10);
    p.color = glm::vec3(.7f,0.7f,0.7f);
    planes.push_back(p);

//    Plane * rightWall = new Plane("rightWall");
    p.position = glm::vec3(15,0,0);
    p.pointA = glm::vec3(15,1,0);
    p.pointB = glm::vec3(15,1,1);
    p.color = glm::vec3(1,0,0);
    planes.push_back(p);


//    Plane * leftWall = new Plane("leftWall");
    p.position = glm::vec3(-15,0,0);
    p.pointA = glm::vec3(-15,1,1);
    p.pointB = glm::vec3(-15,1,0);
    p.color = glm::vec3(0,0,1);
    planes.push_back(p);
    return planes;
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), _eye(glm::vec3(0,0,-30))
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

    // We send 16 float to make memory aligned
    for(size_t i =0 ; i < _numPlanes; i++)
    {
        const dwg::Plane &plane = planes[i];
        fPlanes.push_back(plane.position.x); // Pos
        fPlanes.push_back(plane.position.y);
        fPlanes.push_back(plane.position.z);
        fPlanes.push_back(0.0f);
        fPlanes.push_back(plane.pointA.x); // PA
        fPlanes.push_back(plane.pointA.y);
        fPlanes.push_back(plane.pointA.z);
        fPlanes.push_back(0.0f);
        fPlanes.push_back(plane.pointB.x); // PB
        fPlanes.push_back(plane.pointB.y);
        fPlanes.push_back(plane.pointB.z);
        fPlanes.push_back(0.0f);
        fPlanes.push_back(plane.color.r);  // Color
        fPlanes.push_back(plane.color.g);
        fPlanes.push_back(plane.color.b);
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
        _updateWithCL();
    });

    _glView->setFixedSize(textureWidth, textureHeight);

    _drawButton = new QPushButton("Draw");
    QObject::connect(_drawButton, &QPushButton::clicked,[=]
    {
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

        size_t localSize = sizeof(float)*16*12*16;

        clContext->dispatchKernel("rayTracing", range, {&_sharedTextureBufferId,
                                                        &_spheresBufferId, &_numSpheres,
                                                        &_planesBufferId, &_numPlanes,
                                                        &_lightsBufferId, &_numLights,
                                                        KernelArg::getShared(localSize),
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
