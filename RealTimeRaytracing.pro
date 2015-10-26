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

SOURCES += *.cpp

HEADERS  += *.h
HEADERS  += *.hpp

#this script is useful to test build with mac os x
#include(opencl_build.pri)

macx
{
#LIBS += -framework OpenCL
}

win32
{

AMD
LIBS += $$_PRO_FILE_PWD_/AMD/lib_x86_64/libOpenCL.a
INCLUDEPATH += $$_PRO_FILE_PWD_/AMD/include

}



macx
{
#QMAKE_MAC_SDK = macosx10.11
}

OTHER_FILES += \
    cl_files/raytracing.cl

RESOURCES += \
    kernels.qrc
