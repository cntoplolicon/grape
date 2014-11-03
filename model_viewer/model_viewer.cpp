#include <iostream>
#include "GL/glus.h"
#include "mesh.hpp"

const float zNear = 1.0f, zFar = 20.0f, fov = 60.0f;

GLUSprogram program;

GLuint modelLocation;
GLuint viewLocation;
GLuint projectionLocation;
Mesh* mesh;


GLuint vertexArray;
GLuint vertexBuffer;
GLuint indexBuffer;

void initProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    glusFileLoadText("./vertex.glsl", &vertexSource);
    glusFileLoadText("./fragment.glsl", &fragmentSource);
    glusProgramBuildFromSource(&program, const_cast<const GLUSchar**>(&vertexSource.text), 0, 0, 0, const_cast<const GLUSchar**>(&fragmentSource.text));

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    modelLocation = glGetUniformLocation(program.program, "modelMatrix");
    viewLocation = glGetUniformLocation(program.program, "viewMatrix");
    projectionLocation = glGetUniformLocation(program.program, "projectionMatrix");

    glUseProgram(program.program);

    GLfloat model[16];
    glusMatrix4x4Identityf(model);
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model);

    GLfloat view[16];
    glusMatrix4x4LookAtf(view, 0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, view);

    GLfloat projection[16];
    glusMatrix4x4Perspectivef(projection, fov, 1.0f, zNear, zFar);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection);

    glUseProgram(0);
}

void initMesh()
{
    mesh = new Mesh("./model/Ship.xml");
}

GLUSboolean init()
{
    initProgram();
    initMesh();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    return GLUS_TRUE;
}

GLUSboolean update(GLUSfloat time)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program.program);
    mesh->render();
    glUseProgram(0);

    return GLUS_TRUE;
}

void reshape (GLUSint width, GLUSint height)
{
    GLfloat projection[16];
    glusMatrix4x4Perspectivef(projection, fov, (GLfloat)width / (GLfloat)height, zNear, zFar);

    glUseProgram(program.program);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection);
    glUseProgram(0);

    glViewport(0, 0, width, height);
}

GLUSvoid terminate(GLUSvoid)
{
    delete mesh;
    mesh = nullptr;
}

int main(int argc, char* argv[])
{
    EGLint eglConfigAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint eglContextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    glusWindowSetInitFunc(init);
    glusWindowSetReshapeFunc(reshape);
    glusWindowSetUpdateFunc(update);
    glusWindowSetTerminateFunc(terminate);

    if (!glusWindowCreate("Main Window", 480, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes)) {
        return -1;
    }

    glusWindowRun();

    return 0;
}
