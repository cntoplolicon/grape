#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "gbuffer.hpp"
#include "lighting.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 1024;

//const Vector3f COLOR_WHITE = {1.0f, 1.0f, 1.0f};
const Vector3f COLOR_RED = {1.0f, 0.0f, 0.0f};
const Vector3f COLOR_GREEN = {0.0f, 1.0f, 0.0f};
const Vector3f COLOR_CYAN = {0.0f, 1.0f, 1.0f};
const Vector3f COLOR_BLUE = {0.0f, 0.0f, 1.0f};

struct GeometryPass
{
    GLuint program;
    GLuint modelMatrix;
    GLuint viewMatrix;
    GLuint projectionMatrix;
    GLuint textureSampler;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        modelMatrix = glGetUniformLocation(program, "modelMatrix");
        viewMatrix = glGetUniformLocation(program, "viewMatrix");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
        textureSampler = glGetUniformLocation(program, "textureSampler");
    }
};

GeometryPass geometryPass;
Mesh *pBoxMesh;
Mesh *pQuadMesh;
Mesh *pSphereMesh;
GBuffer *pGBuffer;
Camera camera;

Vector3f boxPositions[5];
PointLight pointLights[3];
DirectionalLight directionLight;

void initBoxPositions()
{
    boxPositions[0] = {0.0f, 0.0f, 5.0f};
    boxPositions[1] = {6.0f, 1.0f, 10.0f};
    boxPositions[2] = {-5.0f, -1.0f, 12.0f};
    boxPositions[3] = {4.0f, 4.0f, 15.0f};
    boxPositions[4] = {-4.0f, 2.0f, 20.0f};
}

void initLights()
{
    directionLight.ambientIntensity = 0.1f;
    directionLight.color = COLOR_CYAN;
    directionLight.diffuseIntensity = 0.5f;
    directionLight.direction = {1.0f, 0.0f, 0.0f};

    pointLights[0].diffuseIntensity = 0.2f;
    pointLights[0].color = COLOR_GREEN;
    pointLights[0].position = {0.0f, 1.5f, 5.0f};
    pointLights[0].attenuation.constant = 0.0f;
    pointLights[0].attenuation.linear = 0.0f;
    pointLights[0].attenuation.quadratic = 0.3f;

    pointLights[1].diffuseIntensity = 0.2f;
    pointLights[1].color = COLOR_RED;
    pointLights[1].position = {2.0f, 0.0f, 5.0f};
    pointLights[1].attenuation.constant = 0.0f;
    pointLights[1].attenuation.linear = 0.0f;
    pointLights[1].attenuation.quadratic = 0.3f;

    pointLights[2].diffuseIntensity = 0.2f;
    pointLights[2].color = COLOR_BLUE;
    pointLights[2].position = {0.0f, 0.0f, 3.0f};
    pointLights[2].attenuation.constant = 0.0f;
    pointLights[2].attenuation.linear = 0.0f;
    pointLights[2].attenuation.quadratic = 0.3f;
}

void initGeometryPass()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./geometry_pass.vs.glsl", &vertexSource);
    glusFileLoadText("./geometry_pass.fs.glsl", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    geometryPass.loadUniforms(glusProgram.program);
}

GLUSboolean init(GLUSvoid)
{
    initBoxPositions();
    initLights();
    initGeometryPass();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    pQuadMesh = new Mesh();
    pQuadMesh->LoadMesh("../content/quad.obj");
    pBoxMesh = new Mesh();
    pBoxMesh->LoadMesh("../content/box.obj");
    pSphereMesh = new Mesh();
    pSphereMesh->LoadMesh("../content/sphere.obj");

    pGBuffer = new GBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    glViewport(0, 0, width, height);
}

void renderGeometryPass()
{
    static float m_scale = 0.0f;
    m_scale += 0.05f;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    pGBuffer->bindForWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(geometryPass.program);

    Matrix4x4f viewMatrix = camera.getMatrix();
    glUniformMatrix4fv(geometryPass.viewMatrix, 1, GL_FALSE, viewMatrix.const_value_ptr());

    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(geometryPass.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    glUniform1i(geometryPass.textureSampler, 0);
    for (int i = 0; i < sizeof(boxPositions) / sizeof(boxPositions[0]); i++) {
        Matrix4x4f modelMatrix = Matrix4x4f::identity().translate(boxPositions[i]).rotatey(m_scale);
        glUniformMatrix4fv(geometryPass.modelMatrix, 1, GL_FALSE, modelMatrix.const_value_ptr());
        pBoxMesh->Render();
    }

    glUseProgram(0);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void beginLightPasses()
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    pGBuffer->bindForReading();
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderLightPass()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGBuffer->bindForReading();

    GLint halfWidth = WINDOW_WIDTH / 2;
    GLint halfHeight = WINDOW_HEIGHT / 2;

    pGBuffer->setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, halfWidth, halfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    pGBuffer->setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, halfHeight, halfWidth, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    pGBuffer->setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, halfWidth, halfHeight, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    pGBuffer->setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, halfWidth, 0, WINDOW_WIDTH, halfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

GLUSboolean update(GLUSfloat time)
{
    renderGeometryPass(); 
    beginLightPasses();
    renderLightPass();

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

GLUSvoid terminate(GLUSvoid)
{
    delete pQuadMesh;
    delete pBoxMesh;
    delete pSphereMesh;
    delete pGBuffer;
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

