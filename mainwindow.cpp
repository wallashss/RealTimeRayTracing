#include "mainwindow.h"

#include <glview.h>
#include <drawables.hpp>
#include <scene.h>

#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

#include <iostream>

#include <glm/gtx/rotate_vector.hpp>

static const int textureWidth = 640;
static const int textureHeight = 480;

static const glm::vec3 ORIGINAL_EYE(0,0,-40);


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)//, _eye(ORIGINAL_EYE)
{
    setMinimumSize(1024, 768);

    QHBoxLayout * hlayout = new QHBoxLayout();

    // Initialize OpenGL View
    _glView = new GLView(textureWidth, textureHeight, [=] (GLView * newGlView) mutable
    {
        // Setup Scene
        dwg::Scene defaultScene;
        defaultScene.spheres = getDefaultSceneSpheres();
        defaultScene.planes = getDefaultScenePlanes();
        defaultScene.lights = getDefaultSceneLights();

        // Initialize Raytracer
        _raytracer = std::make_shared<RayTracing>(defaultScene, newGlView->getBaseTexture(), textureWidth, textureWidth);
        _raytracer->setEye(ORIGINAL_EYE);
    });
    _glView->setFixedSize(textureWidth, textureHeight);


    // Timer for rotation update
    _qtimer = new QTimer(this);
    _qtimer->setInterval(0); // max 60 FPS
    QObject::connect(_qtimer, &QTimer::timeout, [&]
    {
        _raytracer->setEye(glm::rotateY(_raytracer->getEye(), glm::pi<float>()*0.01f));
        _updateScene();
    });


    // Buttons
    QPushButton * drawButton = new QPushButton("Draw");
    QObject::connect(drawButton, &QPushButton::clicked,[=]
    {
        _raytracer->setEye(ORIGINAL_EYE);
        _updateScene();
    });

    QPushButton *rotateButton = new QPushButton("Rotate");
    QObject::connect(rotateButton, &QPushButton::clicked,[=]
    {
        if(!_qtimer->isActive())
        {
            _updateTimer.restart();
            _qtimer->start();
        }
        else
        {
            _qtimer->stop();
        }
    });

    // Populate view
    hlayout->addWidget(_glView);
    hlayout->addWidget(drawButton);
    hlayout->addWidget(rotateButton);

    setLayout(hlayout);
}

void MainWindow::_updateScene()
{
    util::Timer t;

    // Active openGL Context
    _glView->makeCurrent();

    // Raytracing
    _raytracer->update();

    // Finish OpenGL Context
    _glView->doneCurrent();

    // Redraw
    _glView->repaint();

    float elapsedTime = t.elapsedMilliSec();
    setWindowTitle(QString::fromStdString("Rendered: ") + QString::fromStdString(std::to_string(elapsedTime)) + QString(" ms") +
                   QString(" (") + QString::fromStdString(std::to_string((1.0f/elapsedTime)*1e3f)) + QString(" FPS)"));
}


MainWindow::~MainWindow()
{

}
