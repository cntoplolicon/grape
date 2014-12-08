#ifndef _BILLBOARD_PROGRAM_H_
#define _BILLBOARD_PROGRAM_H_

#include "GL/glus.h"

struct BillboardProgram
{
    GLuint program;
    GLuint modelViewMatrix;
    GLuint projectionMatrix;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
    }
};

#endif

