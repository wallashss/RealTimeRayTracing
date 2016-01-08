#include "glview.h"

#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

GLView::GLView(int textureWidth, int textureHeight, std::function<void(GLView *)> initCallback) : QOpenGLWidget(), _textureWidth(textureWidth), _textureHeight(textureHeight)
{
#if __APPLE__
    QSurfaceFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile(QSurfaceFormat::OpenGLContextProfile::CompatibilityProfile);
    glFormat.setSwapBehavior(QSurfaceFormat::SwapBehavior::SingleBuffer);
    glFormat.setSwapInterval(0);
    setFormat(glFormat);
#endif

    _initCallback =  initCallback;
}

GLuint GLView::createTexture(unsigned int width, unsigned int height)
{
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    return textureId;
}

void GLView::setPaintCallback(std::function<void(GLView*)> callback)
{
    _paintCallback = callback;
}

void GLView::setBaseTexture(GLuint newTextureId)
{
    _glTexture = newTextureId;
}

GLuint GLView::getBaseTexture() const
{
    return _glTexture;
}

void GLView::paintGL()
{
    _checkErrors("before draw");
//    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, _glTexture);
    glEnable(GL_TEXTURE_2D);

    if(_paintCallback)
    {
        _paintCallback(this);
    }

    glBegin(GL_QUADS);
    {
       glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
       glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
       glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
       glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    _checkErrors("after draw");
}

void GLView::initializeGL()
{
    // Initialize OpenGL Functions
    initializeOpenGLFunctions();

    glViewport(0, 0, width(), height());

    _checkErrors("Before initializeGL");
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    glGenTextures(1, &_glTexture);
    // In windows, here width and height
    if(_glTexture)
    {
        // Chess dummy texture
        int totalBytes = _textureWidth*_textureHeight*4;
        unsigned char * dummyTexture = new unsigned char[totalBytes];


        for(int i = 0 ; i < _textureHeight; i++)
        {
            for(int j = 0 ; j < _textureWidth; j++)
            {
                int tilex = i / 40 ;
                int tiley = j / 40 ;

                int index = i* (_textureWidth*4) + (j*4);

                if((tilex % 2 == 0 && tiley % 2 == 0) || (tilex % 2 != 0 && tiley % 2 !=0))
                {
                    dummyTexture[index +0] = 128;
                    dummyTexture[index +1] = 128;
                    dummyTexture[index +2] = 128;
                    dummyTexture[index +3] = 255;
                }
                else
                {
                    dummyTexture[index +0] = 0;
                    dummyTexture[index +1] = 0;
                    dummyTexture[index +2] = 128;
                    dummyTexture[index +3] = 255;
                }
            }
        }

        glBindTexture(GL_TEXTURE_2D, _glTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _textureWidth, _textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dummyTexture);
        delete [] dummyTexture;
    }
    else
    {
        std::cout << "Error loading texture " << std::endl;
    }

    if(_initCallback)
    {
        _initCallback(this);
    }
    _checkErrors("After initializeGL");
}

void GLView::_checkErrors(const std::string & snippet = "")
{
    for(GLenum currError = glGetError(); currError != GL_NO_ERROR; currError = glGetError())
    {
        std::cout << "Error '" << snippet << "' " <<  currError << std::endl;
    }
}
