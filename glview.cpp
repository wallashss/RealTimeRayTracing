#include "glview.h"

#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

static QGLFormat getFormat()
{
    QGLFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QGLFormat::CompatibilityProfile );
    glFormat.setSampleBuffers( false );
    glFormat.setDoubleBuffer(false);
    glFormat.setSwapInterval(0);

    return glFormat;
}


static void checkErrors(const std::string & snippet = "")
{
    for(GLenum currError = glGetError(); currError != GL_NO_ERROR; currError = glGetError())
    {
        std::cout << "Error '" << snippet << "' " <<  currError << std::endl;
    }
}

GLView::GLView() : QGLWidget(getFormat())
{

}

void GLView::paintGL()
{
    checkErrors("before draw");
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, _textureBuffer);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);


    {
        glColor3f(1.0f, 1.0f, 1.0f);
//        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-0.5, 0.5f, 0);
        glTexCoord2f(0, 1.0f);

//        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.5f, 0.5f, 0);
        glTexCoord2f(1.0f, 1.0f);

//        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.5, -0.5f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);

//        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, 0);
        glTexCoord2f(0.0f, 0.0f);
    }

    glEnd();


    checkErrors("after draw");
}

void GLView::initializeGL()
{
    checkErrors("Before initializeGL");
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    glGenTextures(1, &_textureBuffer);

    if(_textureBuffer)
    {
        int totalBytes = width()*height()*3;
        unsigned char * dummyTexture = new unsigned char[totalBytes];

        for(int i = 0 ; i < height() ; i++)
        {
            for(int j = 0 ; j < width(); j++)
            {
                int tilex = i / 32 ;
                int tiley = j / 24 ;

                int index = i* (width()*3) + (j*3);

                if((tilex % 2 == 0 && tiley % 2 == 0) || (tilex % 2 != 0 && tiley % 2 !=0))
                {
                    dummyTexture[index +0] = 255;
                    dummyTexture[index +1] = 255;
                    dummyTexture[index +2] = 255;
                }
                else
                {
                    dummyTexture[index +0] = 0;
                    dummyTexture[index +1] = 0;
                    dummyTexture[index +2] = 0;
                }
            }
        }

        glBindTexture(GL_TEXTURE_2D, _textureBuffer);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, dummyTexture);
        delete [] dummyTexture;
    }
    else
    {
        std::cout << "Error loading texture " << std::endl;
    }
    checkErrors("After initializeGL");
}
