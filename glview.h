#pragma once

#include <QGLWidget>

class GLView : public QGLWidget
{
public:
    GLView();

protected:

    void paintGL();

    void initializeGL();

private:

    GLuint _textureBuffer;

};

