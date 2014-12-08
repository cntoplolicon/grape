#ifndef _RANDOM_TEXTURE_H_
#define _RANDOM_TEXTURE_H_

#include "GL/glus.h"
#include "math.hpp"

struct RandomTexture
{
    GLuint textureObj = 0;

    RandomTexture(int size)
    {
        Vector3f* pRandomData = new Vector3f[size];
        for (int i = 0 ; i < size ; i++) {
            pRandomData[i].x = rand() * 1.0f / RAND_MAX;
            pRandomData[i].y = rand() * 1.0f / RAND_MAX;
            pRandomData[i].z = rand() * 1.0f / RAND_MAX;
        }

        glGenTextures(1, &textureObj);
        glBindTexture(GL_TEXTURE_1D, textureObj);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0, GL_RGB, GL_FLOAT, pRandomData);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);    

        delete [] pRandomData;
    }

    ~RandomTexture()
    {
        if (textureObj) {
            glDeleteTextures(1, &textureObj);
        }
    }

    void Bind(GLenum textureUnit)
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_1D, textureObj);
    }
};

#endif

