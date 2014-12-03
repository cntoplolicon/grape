#ifndef __MATH__H__
#define __MATH__H__

#include "GL/glus.h"

struct Vector3f
{
    GLfloat x, y, z;
    GLfloat * value_ptr()
    {
        return &x;
    }
};

#endif


