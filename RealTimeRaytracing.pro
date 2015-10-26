#-------------------------------------------------
#
# Project created by QtCreator 2015-10-25T14:56:30
#
#-------------------------------------------------

QT       += core opengl

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RealTimeRaytracing
TEMPLATE = app

INCLUDEPATH += glm

SOURCES += main.cpp\
        mainwindow.cpp \
    glview.cpp \
    raytracing.cpp \
    clcontextwrapper.cpp

HEADERS  += mainwindow.h \
    glview.h \
    raytracing.h \
    clcontextwrapper.h \
    util.hpp

#include(opencl_build.pri)
LIBS += -framework OpenCL

macx
{
QMAKE_MAC_SDK = macosx10.11
}

OTHER_FILES += \
    cl_files/raytracing.cl

RESOURCES += \
    kernels.qrc
