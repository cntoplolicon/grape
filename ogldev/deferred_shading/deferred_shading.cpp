#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "gbuffer.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 1024;

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
Mesh *pMesh;
GBuffer *pGBuffer;
Camera camera;

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
    initGeometryPass();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    pMesh = new Mesh();
    pMesh->LoadMesh("../content/phoenix_ugv.md2");

    pGBuffer = new GBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

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

    pGBuffer->bindForWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(geometryPass.program);

    Matrix4x4f modelMatrix = Matrix4x4f::identity().translate({-0.8f, -1.0f, 12.0f}).rotatey(m_scale).scale({0.1f, 0.1f, 0.1f});
    glUniformMatrix4fv(geometryPass.modelMatrix, 1, GL_FALSE, modelMatrix.const_value_ptr());

    Matrix4x4f viewMatrix = camera.getMatrix();
    glUniformMatrix4fv(geometryPass.viewMatrix, 1, GL_FALSE, viewMatrix.const_value_ptr());

    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(geometryPass.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    glUniform1i(geometryPass.textureSampler, 0);

    pMesh->Render();

    glUseProgram(0);
}

void renderLightPass()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

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
    delete pMesh;
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

