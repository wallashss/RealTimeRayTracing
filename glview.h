#pragma once

#include <QOpenGLWidget>

#include <QOpenGLFunctions_3_3_Compatibility>
#include <QOpenGLFunctions_3_3_Core>

#include <QOpenGLFunctions>
#include <QGLWidget>

class GLView : public QOpenGLWidget , public QOpenGLFunctions_3_3_Compatibility
{
public:
    GLView();

protected:

    void paintGL();

    void initializeGL();

private:
    void _checkErrors(const std::string & snippet);

private:

    GLuint _textureBuffer;

};

