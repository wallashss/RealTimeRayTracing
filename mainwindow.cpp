#include "mainwindow.h"

#include <glview.h>

#include <QBoxLayout>
#include <QFile>
#include <QTextStream>

#include <iostream>

#include <clcontextwrapper.h>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{

    setMinimumSize(640, 480);

    QHBoxLayout * hlayout = new QHBoxLayout();

    GLView * glView = new GLView();


    QFile vertexFile(":/cl_files/raytracing.cl");

    if(!vertexFile.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        std::cout << "Failed to load cl file" << std::endl;
        return;
    }
    QTextStream vertexTextStream(&vertexFile);
    QString clSource = vertexTextStream.readAll();

//    std::cout << clSource.toStdString() << std::endl;

    CLContextWrapper context;
    context.createContext(DeviceType::GPU_DEVICE);
    context.createProgramFromSource(clSource.toStdString());
    context.prepareKernel("testKernel");

    int a1[15], a2[10];

    for(int i = 0 ; i < 10 ;i++)
    {
        a1[i] = i;
    }

    auto id1 = context.createBuffer(sizeof(int)*10, a1, BufferType::READ_AND_WRITE);
    auto id2 = context.createBuffer(sizeof(int)*10, nullptr, BufferType::READ_AND_WRITE);

    NDRange range;
    range.workDim = 1;
    range.globalOffset[0] = 0;
    range.globalSize[0] = 10;
    range.localSize[0] = 10;

    KernelArg arg1;
    arg1.data = &id1;
    arg1.type = KernelArgType::GLOBAL;

    KernelArg arg2;
    arg2.data = &id2;
    arg2.type = KernelArgType::GLOBAL;

    context.dispatchKernel("testKernel", range, {arg1, arg2});

    context.dowloadFromBuffer(id2, sizeof(int)*10, a2);

    for(int i = 0 ; i < 10 ;i++)
    {
        std::cout << a1[i] << " " << a2[i]<< std::endl;
    }

    hlayout->addWidget(glView);
    setLayout(hlayout);

}

MainWindow::~MainWindow()
{


}
