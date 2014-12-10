#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "lighting_program.hpp"

const int WINDOW_WIDTH = 1680;
const int WINDOW_HEIGHT = 1050;

DirectionalLight directionalLight;

Mesh *pMesh;
Texture *colorMap;
float tessellationLevel = 1.0f;

struct TesselationProgram : public LightingProgram<0, 0>
{
    GLuint tessellationLevel;
    virtual void loadUniforms(GLuint program)
    {
        LightingProgram::loadUniforms(program);
        tessellationLevel = glGetUniformLocation(program, "tessellationLevel");
    }
};

TesselationProgram program;
Camera camera;

void initProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile controlSource;
    GLUStextfile evaluationSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./lighting.vs.glsl", &vertexSource);
    glusFileLoadText("./lighting.cs.glsl", &controlSource);
    glusFileLoadText("./lighting.es.glsl", &evaluationSource);
    glusFileLoadText("./lighting.fs.glsl", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            const_cast<const GLUSchar **>(&controlSource.text), const_cast<const GLUSchar **>(&evaluationSource.text), 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    program.loadUniforms(glusProgram.program);
}

GLUSboolean init(GLUSvoid)
{
    initProgram();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    pMesh = new Mesh();
    pMesh->LoadMesh("../content/monkey.obj");
    glActiveTexture(GL_TEXTURE0);
    colorMap = new Texture(GL_TEXTURE_2D, "../content/white.png");

    camera.setPosition(0.0f, 1.5f, -6.5f);
    camera.setDirection(0.0f, -0.2f, 1.0f);
    camera.setUpDireciton(0.0, 1.0f, 0.0f);       

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    directionalLight.color = {1.0f, 1.0f, 1.0f};
    directionalLight.ambientIntensity = 0.1f;
    directionalLight.diffuseIntensity = 0.9f;

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

    // model view
    Matrix4x4f modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.translate({-3.0f, 0.0f, 0.0f});
    modelViewMatrix = modelViewMatrix.rotatex(-90.0f);
    modelViewMatrix = modelViewMatrix.scale({2.0f, 2.0f, 2.0f});
    Matrix4x4f viewMatrix = camera.getMatrix();
    modelViewMatrix = viewMatrix * modelViewMatrix;
    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // model view for normal
    modelViewMatrix = modelViewMatrix.inverse().transpose();
    glUniformMatrix4fv(program.modelViewMatrixForNormal, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // projection
    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    // lights
    directionalLight.direction = viewMatrix * Vector3f{1.0f, -1.0f, 0.0f};
    program.setDirectionalLight(directionalLight);
    program.setPointLights(nullptr, 0);
    program.setSpotLights(nullptr, 0); 

    // specular lighting
    glUniform1f(program.specular.specularIntensity, 0.0f);
    glUniform1f(program.specular.shiness, 0.0f);

    // texture
    glUniform1i(program.textureSampler, 0);
    colorMap->Bind(GL_TEXTURE0);

    // tessellation level
    glUniform1f(program.tessellationLevel, tessellationLevel);

    pMesh->Render();

    // another mesh
    modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.translate({3.0f, 0.0f, 0.0f});
    modelViewMatrix = modelViewMatrix.rotatex(-90.0f);
    modelViewMatrix = modelViewMatrix.scale({2.0f, 2.0f, 2.0f});
    modelViewMatrix = viewMatrix * modelViewMatrix;
    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    glUniform1f(program.tessellationLevel, 1.0f);

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
        case 'z':
            tessellationLevel += 1.0f;
            break;
        case 'x':
            tessellationLevel -= 1.0f;
            break;
        case 'q':
            exit(0);
        default:
            break;
    }
    tessellationLevel = tessellationLevel < 1.0f ? 1.0f : tessellationLevel;
    std::cout << "tessellation level: " << tessellationLevel << std::endl;
}

GLUSvoid mouseMove(GLUSint buttons, GLUSint x, GLUSint y)
{
    camera.onMouse(buttons, x, y);
}

GLUSvoid terminate(GLUSvoid)
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
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 1,
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

