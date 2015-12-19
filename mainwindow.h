#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QPushButton>

#include <clcontextwrapper.h>
#include <memory>
#include <glview.h>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    void _testOpenCL();
    std::shared_ptr<CLContextWrapper> clContext;
    unsigned int _glTexture;
    BufferId _sharedTextureBufferId;

    BufferId _spheresBufferId;
    BufferId _spheresColorsBufferId;

    BufferId _planesBufferId;
    BufferId _planesColorsBufferId;

    BufferId _lightsBufferId;
    BufferId _lightsColorsBufferId;


    QPointer<GLView> _glView;
    QPointer<QPushButton> _drawButton;

};
