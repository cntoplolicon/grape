#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "lighting_program.hpp"

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;                                                      
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1200;

DirectionalLight directionalLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Mesh *pMesh = nullptr;

LightingProgram<MAX_POINT_LIGHTS, MAX_SPOT_LIGHTS> program;
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

    program.loadUniforms(glusProgram.program);
}

GLUSboolean init(GLUSvoid)
{
    initProgram();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    pMesh = new Mesh();
    pMesh->LoadMesh("../Content/phoenix_ugv.md2");

    camera.setPosition(3.0f, 7.0f, -10.0f);
    camera.setDirection(0.0f, -0.2f, 1.0f);
    camera.setUpDireciton(0.0, 1.0f, 0.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    directionalLight.color = {1.0f, 1.0f, 1.0f};
    directionalLight.ambientIntensity = 1.0f;
    directionalLight.diffuseIntensity = 0.01f;
    directionalLight.direction = {1.0f, -1.0f, 0.0f};

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
    static float m_scale = 0.0f;
    m_scale += 0.01f;
    Matrix4x4f modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.translate({0.0f, 0.0f, 10.0f});
    modelViewMatrix = modelViewMatrix.rotatey(m_scale);
    modelViewMatrix = modelViewMatrix.scale({0.1f, 0.1f, 0.1f});
    Matrix4x4f viewMatrix = camera.getMatrix();
    modelViewMatrix = viewMatrix * modelViewMatrix;
    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // model view for normal
    modelViewMatrix = modelViewMatrix.inverse().transpose();
    glUniformMatrix4fv(program.modelViewMatrixForNormal, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // projection
    GLfloat projectionMatrix[16];
    glusMatrix4x4Perspectivef(projectionMatrix, 60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 50.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix);

    // directional light
    program.setDirectionalLight(directionalLight);

    // point light
    static float scale = 0.0f;
    scale += 0.0057f;
    pointLights[0].diffuseIntensity = 0.25f;
    pointLights[0].color = {1.0f, 0.5f, 0.0f};
    pointLights[0].position = {3.0f, 1.0f, 20.0f * (cosf(scale) + 1.0f) / 2.0f};
    pointLights[0].attenuation.linear = 0.1f;
    pointLights[1].diffuseIntensity = 0.25f;
    pointLights[1].color = {0.0f, 0.5f, 1.0f};
    pointLights[1].position = {7.0f, 1.0f, 20.0f * (sinf(scale) + 1.0f) / 2.0f};
    pointLights[1].attenuation.linear = 0.1f;

    program.setPointLights(pointLights, MAX_POINT_LIGHTS);

    // spot light
    spotLights[0].diffuseIntensity = 0.9f;
    spotLights[0].color = {0.0f, 1.0f, 1.0f};
    spotLights[0].position = camera.getPosition();
    spotLights[0].direction = camera.getDirection();
    spotLights[0].attenuation.linear = 0.1f;
    spotLights[0].cutoff = 10.0f;
   
    program.setSpotLights(spotLights, 1); 

    // specular lighting
    glUniform1f(program.specular.specularIntensity, 0.0f);
    glUniform1f(program.specular.shiness, 0.0f);

    // texture
    glUniform1i(program.textureSampler, 0);

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
            directionalLight.ambientIntensity -= 0.05f;
            break;
        case 'x':
            directionalLight.ambientIntensity += 0.05f;
            break;
        case 'c':
            directionalLight.diffuseIntensity -= 0.05f;
            break;
        case 'v':
            directionalLight.diffuseIntensity += 0.05f;
            break;
        case 'q':
            exit(0);
        default:
            break;
    }
    directionalLight.ambientIntensity = clamp(directionalLight.ambientIntensity, 0.0f, 1.0f);
    directionalLight.diffuseIntensity = clamp(directionalLight.diffuseIntensity, 0.0f, 1.0f);
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

