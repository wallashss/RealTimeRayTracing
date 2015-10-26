#pragma once

#include <QGLWidget>

#include <QOpenGLFunctions_3_3_Compatibility>


class GLView : public QGLWidget, public QOpenGLFunctions_3_3_Compatibility
{
public:
    GLView();

protected:

    void paintGL();

    void initializeGL();

private:

    GLuint _textureBuffer;

};

