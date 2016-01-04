#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QPushButton>

#include <clcontextwrapper.h>
#include <memory>
#include <glview.h>

#include <glm/glm.hpp>
#include <QTimer>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    void _updateWithCL();

    void testScan();

private:
    std::shared_ptr<CLContextWrapper> _clContext;
    unsigned int _glTexture;
    BufferId _sharedTextureBufferId;
    BufferId _tempTextureBufferId;

    BufferId _spheresBufferId;
    size_t _numSpheres;

    BufferId _planesBufferId;
    size_t _numPlanes;

    BufferId _lightsBufferId;
    size_t _numLights;

    BufferId _raysBufferId;
    BufferId _pixelsBufferId;

    glm::vec3 _eye;

    QPointer<GLView> _glView;
    QPointer<QPushButton> _drawButton;

    QPointer<QTimer> _qtimer;

};
