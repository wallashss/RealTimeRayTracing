#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QPushButton>

#include <clcontextwrapper.h>
#include <raytracing.h>
#include <glview.h>

#include <memory>

#include <glm/glm.hpp>

#include <QTimer>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    void _updateScene();

    void _prefixScan(BufferId buffer, int n);
private:

    std::shared_ptr<RayTracing> _raytracer;

    QPointer<GLView> _glView;
    QPointer<QTimer> _qtimer;
};
