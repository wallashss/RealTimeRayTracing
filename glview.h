#pragma once

#include <QOpenGLWidget>

#include <QOpenGLFunctions_3_3_Compatibility>
#include <QOpenGLFunctions>

#include <QGLWidget>

#include <functional>

#if __APPLE__
class GLView : public QOpenGLWidget , public QOpenGLFunctions
#else
class GLView : public QOpenGLWidget , public QOpenGLFunctions_3_3_Compatibility
#endif
{
public:
    GLView(int textureWidth, int textureHeight, std::function<void(GLView*)> initCallback);

    GLuint createTexture(unsigned int width, unsigned int height);

    void setPaintCallback(std::function<void(GLView*)> callback);

    void setBaseTexture(GLuint newTextureId);

    GLuint getBaseTexture() const;

protected:

    void paintGL();

    void initializeGL();

private:
    void _checkErrors(const std::string & snippet);

private:

    GLuint _glTexture;

    int _textureWidth;

    int _textureHeight;

    std::function<void(GLView*)> _initCallback;

    std::function<void(GLView*)> _paintCallback;

};

