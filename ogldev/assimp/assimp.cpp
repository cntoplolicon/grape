#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "lighting_program.hpp"

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;                                                      
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1200;

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

DirectionalLight directionalLight = {
    {{1.0f, 1.0f, 1.0f}, 0.01f, 0.01f},
    {1.0f, -1.0f, 0.0f}
};
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

GLuint indexBufferObject;
GLuint vertexBufferObject;
GLuint vertexArrayObject;

Texture* pTexture = nullptr;

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
    camera.setPosition(5.0f, 1.0f, -3.0f);
    camera.setDirection(0.0f, 0.0f, 1.0f);
    camera.setUpDireciton(0.0, 1.0f, 0.0f);

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

    // model view
    Matrix4x4f modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.translate({0.0f, 0.0f, 1.0f});
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
    pointLights[0].base.diffuseIntensity = 0.25f;
    pointLights[0].base.color = {1.0f, 0.5f, 0.0f};
    pointLights[0].position = {3.0f, 1.0f, 20.0f * (cosf(scale) + 1.0f) / 2.0f};
    pointLights[0].attenuation.linear = 0.1f;
    pointLights[1].base.diffuseIntensity = 0.25f;
    pointLights[1].base.color = {0.0f, 0.5f, 1.0f};
    pointLights[1].position = {7.0f, 1.0f, 20.0f * (sinf(scale) + 1.0f) / 2.0f};
    pointLights[1].attenuation.linear = 0.1f;

    program.setPointLights(pointLights, MAX_POINT_LIGHTS);

    // spot light
    spotLights[0].base.base.diffuseIntensity = 0.9f;
    spotLights[0].base.base.color = {0.0f, 1.0f, 1.0f};
    spotLights[0].base.position = camera.getPosition();
    spotLights[0].direction = camera.getDirection();
    spotLights[0].base.attenuation.linear = 0.1f;
    spotLights[0].cutoff = 10.0f;
    spotLights[1].base.base.diffuseIntensity = 0.9f;
    spotLights[1].base.base.color = {1.0f, 1.0f, 1.0f};
    spotLights[1].base.position = {5.0f, 3.0f, 10.0f};
    spotLights[1].direction = {0.0f, -1.0f, 0.0f};
    spotLights[1].base.attenuation.linear = 0.1f;
    spotLights[1].cutoff = 20.0f;
   
    program.setSpotLights(spotLights, MAX_SPOT_LIGHTS); 

    // specular lighting
    glUniform1f(program.specular.specularIntensity, 0.0f);
    glUniform1f(program.specular.shiness, 0.0f);

    glUniform1i(program.textureSampler, 0);
    pTexture->Bind(GL_TEXTURE0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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
        case 'q':
            exit(0);
        default:
            break;
    }
    directionalLight.base.ambientIntensity = glusMathClampf(directionalLight.base.ambientIntensity, 0.0f, 1.0f);
    directionalLight.base.diffuseIntensity = glusMathClampf(directionalLight.base.diffuseIntensity, 0.0f, 1.0f);
}

GLUSvoid mouseMove(GLUSint buttons, GLUSint x, GLUSint y)
{
    camera.onMouse(buttons, x, y);
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

