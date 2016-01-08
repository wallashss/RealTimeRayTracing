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

static const int textureWidth = 800;
static const int textureHeight = 592;

static const glm::vec3 ORIGINAL_EYE(0,0,-20);

static const float ROTATION_SPEED = 0.05f;


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{

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
        float deltaTime = _updateTimer.elapsedSec();
        _updateTimer.restart();
        _raytracer->setEye(glm::rotateY(_raytracer->getEye(), glm::pi<float>()*ROTATION_SPEED*deltaTime));
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
    QVBoxLayout * vlayout = new QVBoxLayout();

    vlayout->addWidget(_glView);

    QHBoxLayout * hLayout = new QHBoxLayout();
    hLayout->addWidget(drawButton);
    hLayout->addWidget(rotateButton);

    vlayout->addLayout(hLayout);
    setLayout(vlayout);
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
