#include <iostream>
#include "GL/glus.h"
#include "camera.hpp"

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

const float vertexData[] = {
    -1.0f, -1.0f, 0.5773f,
    0.0f, -1.0f, -1.15475f,
    1.0f, -1.0f, 0.5773f,
    0.0f, 1.0f, 0.0f,
};
const unsigned int indexData[] = {
    0, 3, 1,
    1, 3, 2,
    2, 3, 0,
    0, 1, 2
};

GLuint indexBufferObject;
GLuint vertexBufferObject;
GLuint vertexArrayObject;

struct Program
{
    GLuint program;
    GLuint position;
    GLuint modelViewMatrix;
    GLuint projectionMatrix;
};
Program program;

Camera camera;

void initProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./shader.vs", &vertexSource);
    glusFileLoadText("./shader.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text), 
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    program.program = glusProgram.program;
    program.position = glGetAttribLocation(program.program, "position");
    program.modelViewMatrix = glGetUniformLocation(program.program, "modelViewMatrix");
    program.projectionMatrix = glGetUniformLocation(program.program, "projectionMatrix");
}

void initVertexBuffers()
{
    glUseProgram(program.program);
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(program.position);
    glVertexAttribPointer(program.position, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

GLUSboolean init(GLUSvoid)
{
    initProgram();
    initVertexBuffers();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    glViewport(0, 0, width, height);
}

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program.program);
    glBindVertexArray(vertexArrayObject);

    static float rotate = 0.0f;
    rotate += 0.1f;
    GLfloat modelMatrix[16];
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, 3.0f);
    glusMatrix4x4RotateRyf(modelMatrix, rotate);
    GLfloat viewMatrix[16];
    camera.GetMatrix(viewMatrix);
    GLfloat modelViewMatrix[16];
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);

    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix);

    GLfloat projectionMatrix[16];
    glusMatrix4x4Perspectivef(projectionMatrix, 60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix);

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    return GLUS_TRUE;
}

GLUSvoid keyboard(GLUSboolean pressed, GLUSint key)
{
    if (!pressed) {
        return;
    }
    camera.OnKey(key);
}

GLUSvoid terminate(GLUSvoid)
{
}

int main(int argc, char* argv[])
{
    EGLint eglConfigAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint eglContextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    glusWindowSetInitFunc(init);
    glusWindowSetReshapeFunc(reshape);
    glusWindowSetUpdateFunc(update);
    glusWindowSetTerminateFunc(terminate);
    glusWindowSetKeyFunc(keyboard);

    if (!glusWindowCreate("Main Window", WINDOW_WIDTH, WINDOW_HEIGHT, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
