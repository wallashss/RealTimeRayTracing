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
    std::shared_ptr<CLContextWrapper> clContext;
    unsigned int _glTexture;
    BufferId _sharedTextureBufferId;

    BufferId _spheresBufferId;
    size_t _numSpheres;

    BufferId _planesBufferId;
    size_t _numPlanes;

    BufferId _lightsBufferId;
    size_t _numLights;

    glm::vec3 _eye;

    QPointer<GLView> _glView;
    QPointer<QPushButton> _drawButton;

    QPointer<QTimer> _qtimer;

};
