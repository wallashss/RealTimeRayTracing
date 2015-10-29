#include "mainwindow.h"

#include <glview.h>

#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

#include <iostream>
#include <memory>


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{

    setMinimumSize(1024, 768);

    QHBoxLayout * hlayout = new QHBoxLayout();

    std::shared_ptr<CLContextWrapper> clContext = std::make_shared<CLContextWrapper>();
    std::shared_ptr<BufferId> textureToUpId = std::make_shared<BufferId>(0);
    std::shared_ptr<GLuint> glTexId = std::make_shared<BufferId>(0);

    GLView * glView = new GLView([=] (GLView * newGlView) mutable
    {
        if(clContext->createContextWithOpengl())
        {
            std::cout << "Successfully created OpenCL context with OpenGL" << std::endl;
        }

        auto textureId = newGlView->createTexture(640, 480);
        auto bufferId = clContext->shareGLTexture(textureId, BufferType::WRITE_ONLY);

        *textureToUpId = bufferId;
        *glTexId = textureId;

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

        clContext->createProgramFromSource(clSource.toStdString());
        clContext->prepareKernel("testKernel");
        clContext->prepareKernel("testTexture");
    });

    glView->setFixedSize(640, 480);

    QPushButton *drawButton = new QPushButton("Draw");
    QObject::connect(drawButton, &QPushButton::clicked,[=]
    {
        glView->context()->makeCurrent(nullptr);
        glView->setBaseTexture(*glTexId);

        NDRange range;
        range.workDim = 2;
        range.globalOffset[0] = 0;
        range.globalOffset[1] = 0;
        range.globalSize[0] = 640;
        range.globalSize[1] = 480;
        range.localSize[0] = 16;
        range.localSize[1] = 12;

        auto tid = *textureToUpId.get();
        clContext->executeSafeAndSyncronized(&tid, 1, [&]
        {
            KernelArg arg1;
            arg1.data = &tid;
            arg1.type = KernelArgType::OPENGL;

            clContext->dispatchKernel("testTexture", range, {arg1});
        });

        glView->context()->doneCurrent();

        glView->repaint();
    });

    hlayout->addWidget(glView);
    hlayout->addWidget(drawButton);

    setLayout(hlayout);
}

MainWindow::~MainWindow()
{

}

void MainWindow::_testOpenCL()
{
    // Check available platforms
    std::cout << "Platforms: " << std::endl;
    auto listOfPlatform = CLContextWrapper::listAvailablePlatforms();
    for(auto str : listOfPlatform)
    {
        std::cout << str << std::endl;
    }

    // Dummy Test
    // Create a array with sequential items and sum +1 to every item in a new array
    QFile kernelSourceFile(":/cl_files/raytracing.cl");

    if(!kernelSourceFile.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        std::cout << "Failed to load cl file" << std::endl;
        return;
    }
    QTextStream vertexTextStream(&kernelSourceFile);
    QString clSource = vertexTextStream.readAll();

    CLContextWrapper context;
    context.createContext(DeviceType::CPU_DEVICE);

    if(!context.hasCreatedContext())
    {
        return;
    }

    context.createProgramFromSource(clSource.toStdString());
    context.prepareKernel("testKernel");

    // Watch out to do not surpass max work group size
    const size_t ITEMS_TO_TEST = 32;
    int a1[ITEMS_TO_TEST], a2[ITEMS_TO_TEST];

    for(size_t i = 0 ; i < ITEMS_TO_TEST ;i++)
    {
        a1[i] = static_cast<int>(i);
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

    context.dowloadArrayFromBuffer(id2, ITEMS_TO_TEST, a2);

    for(size_t i = 0 ; i < ITEMS_TO_TEST ;i++)
    {
        std::cout << a1[i] << " " << a2[i]<< std::endl;
    }
}
