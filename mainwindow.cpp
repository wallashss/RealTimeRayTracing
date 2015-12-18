#include "mainwindow.h"

#include <glview.h>
#include <drawables.h>

#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

#include <iostream>


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{

    setMinimumSize(1024, 768);

    QHBoxLayout * hlayout = new QHBoxLayout();

    clContext = std::make_shared<CLContextWrapper>();
    std::shared_ptr<BufferId> textureToUpId = std::make_shared<BufferId>(0);
    std::shared_ptr<GLuint> glTexId = std::make_shared<BufferId>(0);

    int textureWidth = 640;
    int textureHeight = 480;

    GLView * glView = new GLView([=] (GLView * newGlView) mutable
    {
        if(clContext->createContextWithOpengl())
        {
            std::cout << "Successfully created OpenCL context with OpenGL" << std::endl;
        }

        auto textureId = newGlView->createTexture(textureWidth, textureHeight);
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
        clContext->prepareKernel("rayTracing");
    });


    glView->setFixedSize(textureWidth, textureHeight);

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

        BufferId bid = *(textureToUpId.get());


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
        s.radius = 2.0f;
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

        // Red
        s.position = glm::vec3(-5,-3.5,5.0f);
        s.radius = 2.0f;
        s.color = glm::vec3(1,1,1);
        spheres.push_back(s);

        // Yellow
        s.position = glm::vec3(-5,-3.5,5.0f);
        s.radius = 1.7f;
        s.color = glm::vec3(1,1,0);
        spheres.push_back(s);

        float *fSphere = new float[spheres.size()];
        float *fColorSphere = new float[spheres.size()];

        for(int i =0 ; i  < spheres.size(); i++)
        {
            dwg::Sphere sphere = spheres[i];
            fSphere[i*4+0] = sphere.position.x;
            fSphere[i*4+1] = sphere.position.y;
            fSphere[i*4+2] = sphere.position.z;
            fSphere[i*4+3] = sphere.radius;

            fColorSphere[i*3+0] = sphere.color.r;
            fColorSphere[i*3+1] = sphere.color.g;
            fColorSphere[i*3+2] = sphere.color.b;
        }

        size_t numSpheres = spheres.size();

        clContext->executeSafeAndSyncronized(&bid, 1, [=] ()
        {
            BufferId newBid = bid;
            KernelArg imageArg;
            imageArg.data = static_cast<void*>(&newBid);
            imageArg.type = KernelArgType::OPENGL;

            KernelArg sphereArg;
            auto sphereId = clContext->createBuffer(sizeof(float)*4*numSpheres, fSphere, BufferType::READ_ONLY);
            sphereArg.data = &sphereId;
            sphereArg.type = KernelArgType::GLOBAL;

            KernelArg sphereColorArg;
            auto colorsSphereId = clContext->createBuffer(sizeof(float)*3*numSpheres, fColorSphere, BufferType::READ_ONLY);
            sphereColorArg.data = &colorsSphereId;
            sphereColorArg.type = KernelArgType::GLOBAL;

            KernelArg numSpheresArg;
            int *numSpheresPtr = new int;
            *numSpheresPtr = static_cast<int>(numSpheres);
            numSpheresArg.data = numSpheresPtr;
            numSpheresArg.type = KernelArgType::CONSTANT;
            numSpheresArg.byteSize = sizeof(int);

            KernelArg eyeXArg;
            float *eyeX = new float;
            *eyeX = 0.0f;
            eyeXArg.data = eyeX;
            eyeXArg.type = KernelArgType::CONSTANT;
            eyeXArg.byteSize = sizeof(float);

            KernelArg eyeYArg;
            float *eyeY = new float;
            *eyeY = 0.0f;
            eyeYArg.data = eyeY;
            eyeYArg.type = KernelArgType::CONSTANT;
            eyeYArg.byteSize = sizeof(float);

            KernelArg eyeZArg;
            float *eyeZ = new float;
            *eyeZ = 0.0f;
            eyeZArg.data = eyeZ;
            eyeZArg.type = KernelArgType::CONSTANT;
            eyeZArg.byteSize = sizeof(float);

            KernelArg widthArg;
            int *width = new int;
            *width = textureWidth;
            widthArg.data = width;
            widthArg.type = KernelArgType::CONSTANT;
            widthArg.byteSize = sizeof(int);

            KernelArg heightArg;
            int *height = new int;
            *height = textureHeight;
            heightArg.data = height;
            heightArg.type = KernelArgType::CONSTANT;
            heightArg.byteSize = sizeof(int);

            std::cout << "Before dispatch " << std::endl;


            clContext->dispatchKernel("rayTracing", range, {imageArg, sphereArg, sphereColorArg, numSpheresArg, eyeXArg, eyeYArg, eyeZArg, widthArg, heightArg});
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
