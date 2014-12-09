#ifndef _BILLBOARD_PROGRAM_H_
#define _BILLBOARD_PROGRAM_H_

#include "GL/glus.h"

struct BillboardProgram
{
    GLuint program;
    GLuint modelViewMatrix;
    GLuint projectionMatrix;
    GLuint billboardSize;
    GLuint billboardSampler;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
        billboardSampler = glGetUniformLocation(program, "billboardSampler");
        billboardSize = glGetUniformLocation(program, "billboardSize");
    }
};

#endif

