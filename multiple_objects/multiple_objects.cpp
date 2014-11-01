#include <iostream>
#include "GL/glus.h"

const float zNear = 1.0f, zFar = 3.0f, fov = 80.0f;

GLUSprogram program;

GLuint modelLocation;
GLuint viewLocation;
GLuint projectionLocation;

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
    glusMatrix4x4Identityf(view);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, view);

    GLfloat projection[16];
    glusMatrix4x4Perspectivef(projection, fov, 1.0f, zNear, zFar);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection);

    glUseProgram(0);
}

const int numberOfVertices = 36;

#define RIGHT_EXTENT 0.8f
#define LEFT_EXTENT -RIGHT_EXTENT
#define TOP_EXTENT 0.20f
#define MIDDLE_EXTENT 0.0f
#define BOTTOM_EXTENT -TOP_EXTENT
#define FRONT_EXTENT -1.25f
#define REAR_EXTENT -1.75f

#define GREEN_COLOR 0.75f, 0.75f, 1.0f, 1.0f
#define BLUE_COLOR 	0.0f, 0.5f, 0.0f, 1.0f
#define RED_COLOR 1.0f, 0.0f, 0.0f, 1.0f
#define GREY_COLOR 0.8f, 0.8f, 0.8f, 1.0f
#define BROWN_COLOR 0.5f, 0.5f, 0.0f, 1.0f

const float vertexData[] = {
    //Object 1 positions
    LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
    LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
    RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
    RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,

    LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
    LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
    RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
    RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

    LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
    LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
    LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

    RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
    RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
    RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

    LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
    LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
    RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
    RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

    //	0, 2, 1,
    //	3, 2, 0,

    //Object 2 positions
    TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
    MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
    MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
    TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,

    BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
    MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
    MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
    BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

    TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
    MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
    BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,

    TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
    MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
    BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

    BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
    TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
    TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
    BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

    //Object 1 colors
    GREEN_COLOR,
    GREEN_COLOR,
    GREEN_COLOR,
    GREEN_COLOR,

    BLUE_COLOR,
    BLUE_COLOR,
    BLUE_COLOR,
    BLUE_COLOR,

    RED_COLOR,
    RED_COLOR,
    RED_COLOR,

    GREY_COLOR,
    GREY_COLOR,
    GREY_COLOR,

    BROWN_COLOR,
    BROWN_COLOR,
    BROWN_COLOR,
    BROWN_COLOR,

    //Object 2 colors
    RED_COLOR,
    RED_COLOR,
    RED_COLOR,
    RED_COLOR,

    BROWN_COLOR,
    BROWN_COLOR,
    BROWN_COLOR,
    BROWN_COLOR,

    BLUE_COLOR,
    BLUE_COLOR,
    BLUE_COLOR,

    GREEN_COLOR,
    GREEN_COLOR,
    GREEN_COLOR,

    GREY_COLOR,
    GREY_COLOR,
    GREY_COLOR,
    GREY_COLOR,
};

const GLshort indexData[] =
{
    0, 2, 1,
    3, 2, 0,

    4, 5, 6,
    6, 7, 4,

    8, 9, 10,
    11, 13, 12,

    14, 16, 15,
    17, 16, 14,
};

GLuint vertexBufferObject;
GLuint indexBufferObject;
GLuint vertexArrayObject;

void initVertexBufferObjects()
{
    glGenBuffers(1, &vertexBufferObject);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &indexBufferObject);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initArrayVertexObject()
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glGenVertexArrays(1, &vertexArrayObject);

    glBindVertexArray(vertexArrayObject);

    GLint positionLocation = glGetAttribLocation(program.program, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLint colorLocation = glGetAttribLocation(program.program, "color");
    size_t colorDataOffset = sizeof(float) * 3 * numberOfVertices;
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, 0, (void*)colorDataOffset);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLUSboolean init()
{
    initProgram();
    initVertexBufferObjects();
    initArrayVertexObject();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

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

    glBindVertexArray(vertexArrayObject);

    GLfloat model[16];
    glusMatrix4x4Identityf(model);
    glusMatrix4x4Translatef(model, 0.0f, 0.0f, -1.0f);
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model);
    glDrawElements(GL_TRIANGLES, sizeof(indexData) / sizeof(indexData[0]), GL_UNSIGNED_SHORT, 0);

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model);
    glDrawElementsBaseVertex(GL_TRIANGLES, sizeof(indexData) / sizeof(indexData[0]), GL_UNSIGNED_SHORT, 0, numberOfVertices / 2);

    glBindVertexArray(0);
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
