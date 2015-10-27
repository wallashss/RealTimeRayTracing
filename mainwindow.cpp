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

    std::cout << "Platforms: " << std::endl;
    auto listOfPlatform = CLContextWrapper::listAvailablePlatforms();
    for(auto str : listOfPlatform)
    {
        std::cout << str << std::endl;
    }


    CLContextWrapper context;
    context.createContext(DeviceType::GPU_DEVICE);

    if(!context.hasCreatedContext())
    {
        return;
    }

    // Dummy Test
    // Create a array with sequential items and sum +1 to every item in a new array
    context.createProgramFromSource(clSource.toStdString());
    context.prepareKernel("testKernel");

    // Watch out to do not surpass max work group size
    const size_t ITEMS_TO_TEST = 32;
    int a1[ITEMS_TO_TEST], a2[ITEMS_TO_TEST];

    for(size_t i = 0 ; i < ITEMS_TO_TEST ;i++)
    {
        a1[i] = i;
    }

    auto id1 = context.createBuffer(sizeof(int)*ITEMS_TO_TEST, a1, BufferType::READ_AND_WRITE);
    auto id2 = context.createBuffer(sizeof(int)*ITEMS_TO_TEST, nullptr, BufferType::READ_AND_WRITE);

    NDRange range;
    range.workDim = 1;
    range.globalOffset[0] = 0;
    range.globalSize[0] = ITEMS_TO_TEST;
    range.localSize[0] = ITEMS_TO_TEST;

    KernelArg arg1;
    arg1.data = &id1;
    arg1.type = KernelArgType::GLOBAL;

    KernelArg arg2;
    arg2.data = &id2;
    arg2.type = KernelArgType::GLOBAL;

    context.dispatchKernel("testKernel", range, {arg1, arg2});

    context.dowloadFromBuffer(id2, sizeof(int)*ITEMS_TO_TEST, a2);

    for(size_t i = 0 ; i < ITEMS_TO_TEST ;i++)
    {
        std::cout << a1[i] << " " << a2[i]<< std::endl;
    }

    hlayout->addWidget(glView);
    setLayout(hlayout);
}

MainWindow::~MainWindow()
{


}
