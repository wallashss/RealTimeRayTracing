#pragma once

#include <QOpenGLWidget>

#include <QOpenGLFunctions_3_3_Compatibility>

#include <QOpenGLFunctions>


class GLView : public QOpenGLWidget , public QOpenGLFunctions
{
public:
    GLView();

protected:

    void paintGL();

    void initializeGL();

private:

    GLuint _textureBuffer;

};

