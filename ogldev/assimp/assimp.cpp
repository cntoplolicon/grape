#include <iostream>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

const float vertexData[] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 20.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 20.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    10.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f
};
const unsigned int indexData[] = {
    0, 3, 1,
    1, 3, 2,
    2, 3, 0,
    0, 1, 2
};

struct BaseLight
{
    GLfloat color[3];
    GLfloat ambientIntensity;
    GLfloat diffuseIntensity;
};

struct DirectionalLight
{
    BaseLight base;
    GLfloat direction[3];
};

DirectionalLight directionalLight = {
    {{1.0f, 1.0f, 1.0f}, 0.01f, 0.01f},
    {1.0f, -1.0f, 0.0f}
};

GLuint indexBufferObject;
GLuint vertexBufferObject;
GLuint vertexArrayObject;

Texture* pTexture = nullptr;

struct Program
{
    GLuint program;
    GLuint position;
    GLuint texCoord;
    GLuint normal;
    GLuint modelViewMatrix;
    GLuint modelViewMatrixForNormal;
    GLuint projectionMatrix;
    GLuint textureSampler;

    struct 
    {
        GLuint color;
        GLuint ambientIntensity;
        GLuint diffuseIntensity;
        GLuint direction;
    } directionalLight;

    struct
    {
        GLuint specularIntensity;
        GLuint shiness;
    } material;
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
    program.texCoord = glGetAttribLocation(program.program, "texCoord");
    program.normal = glGetAttribLocation(program.program, "normal");
    program.modelViewMatrix = glGetUniformLocation(program.program, "modelViewMatrix");
    program.modelViewMatrixForNormal = glGetUniformLocation(program.program, "modelViewMatrixForNormal");
    program.projectionMatrix = glGetUniformLocation(program.program, "projectionMatrix");
    program.textureSampler = glGetUniformLocation(program.program, "textureSampler");
    program.directionalLight.color = glGetUniformLocation(program.program, "directionalLight.base.color");
    program.directionalLight.ambientIntensity = glGetUniformLocation(program.program, "directionalLight.base.ambientIntensity");
    program.directionalLight.diffuseIntensity = glGetUniformLocation(program.program, "directionalLight.base.diffuseIntensity");
    program.directionalLight.direction = glGetUniformLocation(program.program, "directionalLight.direction");
    program.material.specularIntensity = glGetUniformLocation(program.program, "specularIntensity");
    program.material.shiness = glGetUniformLocation(program.program, "shiness");
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
    glVertexAttribPointer(program.position, 3, GL_FLOAT, GL_FALSE, 32, 0);
    glEnableVertexAttribArray(program.texCoord);
    glVertexAttribPointer(program.texCoord, 2, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<const GLvoid *>(12));
    glEnableVertexAttribArray(program.normal);
    glVertexAttribPointer(program.normal, 3, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<const GLvoid *>(20));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glDisableVertexAttribArray(program.position);
    glDisableVertexAttribArray(program.texCoord);
    glDisableVertexAttribArray(program.normal);
}

GLUSboolean init(GLUSvoid)
{
    initProgram();
    initVertexBuffers();

    pTexture = new Texture(GL_TEXTURE_2D, "../content/test.png");
    camera.SetPosition(5.0f, 1.0f, -3.0f);
    camera.SetDirection(0.0f, 0.0f, 1.0f);
    camera.SetUpDireciton(0.0, 1.0f, 0.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

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

    // model view
    GLfloat modelMatrix[16];
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, 1.0f);
    GLfloat viewMatrix[16];
    camera.GetMatrix(viewMatrix);
    GLfloat modelViewMatrix[16];
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix);

    // model view for normal
    glusMatrix4x4Inversef(modelViewMatrix);
    glusMatrix4x4Transposef(modelViewMatrix);
    glUniformMatrix4fv(program.modelViewMatrixForNormal, 1, GL_FALSE, modelViewMatrix);

    // projection
    GLfloat projectionMatrix[16];
    glusMatrix4x4Perspectivef(projectionMatrix, 60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 50.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix);

    // directional light
    glUniform3fv(program.directionalLight.color, 1, directionalLight.base.color);
    glUniform1f(program.directionalLight.ambientIntensity, directionalLight.base.ambientIntensity);
    glUniform1f(program.directionalLight.diffuseIntensity, directionalLight.base.diffuseIntensity);
    glUniform3fv(program.directionalLight.direction, 1, directionalLight.direction);
    
    // specular lighting
    glUniform1f(program.material.specularIntensity, 1.0f);
    glUniform1f(program.material.shiness, 2.0f);

    glUniform1i(program.textureSampler, 0);
    pTexture->Bind(GL_TEXTURE0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);

    return GLUS_TRUE;
}

GLUSvoid keyboard(GLUSboolean pressed, GLUSint key)
{
    camera.OnKey(pressed, key);
    if (!pressed) {
        return;
    }
    switch (key) {
        case 'z':
            directionalLight.base.ambientIntensity -= 0.05f;
            break;
        case 'x':
            directionalLight.base.ambientIntensity += 0.05f;
            break;
        case 'c':
            directionalLight.base.diffuseIntensity -= 0.05f;
            break;
        case 'v':
            directionalLight.base.diffuseIntensity += 0.05f;
            break;
        default:
            break;
    }
    directionalLight.base.ambientIntensity = glusMathClampf(directionalLight.base.ambientIntensity, 0.0f, 1.0f);
    directionalLight.base.diffuseIntensity = glusMathClampf(directionalLight.base.diffuseIntensity, 0.0f, 1.0f);
}

GLUSvoid mouseMove(GLUSint buttons, GLUSint x, GLUSint y)
{
    camera.OnMouse(buttons, x, y);
}

GLUSvoid terminate(GLUSvoid)
{
    delete pTexture;
}

int main(int argc, char* argv[])
{
    MagickCore::MagickCoreGenesis(*argv, Magick::MagickFalse);    
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
    glusWindowSetMouseMoveFunc(mouseMove);

    if (!glusWindowCreate("Main Window", WINDOW_WIDTH, WINDOW_HEIGHT, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}

