#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "lighting_program.hpp"
#include "mesh.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 1024;

struct SilhouetteProgram
{
    GLuint program;

    GLuint modelViewMatrix;
    GLuint projectionMatrix;
    GLuint cameraSpaceLightPosition;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
        cameraSpaceLightPosition = glGetUniformLocation(program, "cameraSpaceLightPosition");
    }
};

LightingProgram<0, 0> program;
SilhouetteProgram silhouetteProgram;
DirectionalLight directionalLight;
Camera camera;
Mesh *pMesh;

void initLightingProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./lighting.vs.glsl", &vertexSource);
    glusFileLoadText("./lighting.fs.glsl", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    program.loadUniforms(glusProgram.program);
}

void initSilhouetteProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./silhouette.vs.glsl", &vertexSource);
    glusFileLoadText("./silhouette.gs.glsl", &geometrySource);
    glusFileLoadText("./silhouette.fs.glsl", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, const_cast<const GLUSchar **>(&geometrySource.text), 
            const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    silhouetteProgram.loadUniforms(glusProgram.program);
}

GLUSboolean init(GLUSvoid)
{
    initLightingProgram();
    initSilhouetteProgram();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    pMesh = new Mesh();
    pMesh->LoadMesh("../content/box.obj", true);

    camera.setPosition(0.0f, 4.0f, -7.0f);
    camera.setDirection(0.0f, 0.0f, 1.0f);
    camera.setUpDireciton(0.0, 1.0f, 0.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    directionalLight.color = {1.0f, 1.0f, 1.0f};
    directionalLight.ambientIntensity = 0.55f;
    directionalLight.diffuseIntensity = 0.9f;

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    glViewport(0, 0, width, height);
}

void renderBox()
{

}

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program.program);

    // model view
    Matrix4x4f modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.translate({0.0f, 2.0f, 0.0f});
    Matrix4x4f viewMatrix = camera.getMatrix();
    modelViewMatrix = viewMatrix * modelViewMatrix;
    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // model view for normal
    modelViewMatrix = modelViewMatrix.inverse().transpose();
    glUniformMatrix4fv(program.modelViewMatrixForNormal, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // projection
    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    // lighting
    directionalLight.direction = viewMatrix * Vector3f{1.0f, 0.0f, 0.0f};
    program.setDirectionalLight(directionalLight);
    program.setPointLights(nullptr, 0);
    program.setSpotLights(nullptr, 0); 
    glUniform1f(program.specular.specularIntensity, 0.0f);
    glUniform1f(program.specular.shiness, 0.0f);

    // texture
    glUniform1i(program.textureSampler, 0);

    pMesh->Render();

    glUseProgram(0);

    // silhouette program
    glUseProgram(silhouetteProgram.program);

    // modelview
    glUniformMatrix4fv(silhouetteProgram.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // projection
    glUniformMatrix4fv(silhouetteProgram.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    // light position
    glUniform3fv(silhouetteProgram.cameraSpaceLightPosition, 1, (modelViewMatrix * Vector3f{0.0f, 10.0f, 0.0f}).const_value_ptr());

    glLineWidth(5.0f);

    pMesh->Render();

    glUseProgram(0);

    return GLUS_TRUE;
}

GLUSvoid keyboard(GLUSboolean pressed, GLUSint key)
{
    camera.onKey(pressed, key);
    if (!pressed) {
        return;
    }
    switch (key) {
        case 'q':
            exit(0);
        default:
            break;
    }
}

GLUSvoid mouseMove(GLUSint buttons, GLUSint x, GLUSint y)
{
    camera.onMouse(buttons, x, y);
}

GLUSvoid terminateGLUS(GLUSvoid)
{
    delete pMesh;
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
    glusWindowSetTerminateFunc(terminateGLUS);
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

