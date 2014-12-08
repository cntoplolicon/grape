#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "lighting_program.hpp"
#include "mesh.hpp"
#include "random_texture.hpp"
#include "particle_update_program.hpp"
#include "billboard_program.hpp"

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1200;

const int RANDOM_TEXTURE_SIZE = 1000;
const int MAX_PARTICLES =  1000;

const int PARTICLE_TYPE_LAUNCHER =  0.0f;


LightingProgram<0, 0> lightingProgram;
ParticleUpdateProgram particleUpdateProgram;
BillboardProgram billboardProgram;
Camera camera;
Mesh *pGroundMesh = nullptr;
Texture *pNormalMap = nullptr;
Texture *pTexture = nullptr;
RandomTexture *pRandomTexture = nullptr;
DirectionalLight directionalLight;
GLuint transformFeedbacks[2];
GLuint vertexBuffers[2];

struct Particle
{
    float type;
    Vector3f position;
    Vector3f velocity;
    float lifetime;
};
int currVB = 0;
int currTFB = 0;
bool isfirst = true;

void initParticleSystem()
{
    Particle particles[MAX_PARTICLES];
    memset(particles, 0, sizeof(particles));

    particles[0] = {PARTICLE_TYPE_LAUNCHER, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0001f, 0.0f}, 0.0f};

    glGenTransformFeedbacks(2, transformFeedbacks);
    glGenBuffers(2, vertexBuffers);
    for (int i = 0; i < 2; i++) {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbacks[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBuffers[i]);
    }
}

void initLightingProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./lighiing.vs", &vertexSource);
    glusFileLoadText("./lighting.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    lightingProgram.loadUniforms(glusProgram.program);
}

void initBillbardProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./billboard.vs", &vertexSource);
    glusFileLoadText("./billboard.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    billboardProgram.loadUniforms(glusProgram.program);
}

void initParticleUpdateProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./ps_update.vs", &vertexSource);
    glusFileLoadText("./ps_update.gs", &geometrySource);
    glusFileLoadText("./ps_update.fs", &fragmentSource);
    glusProgramCreateFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, const_cast<const GLUSchar **>(&geometrySource.text), const_cast<const GLUSchar **>(&fragmentSource.text));
    const GLchar* Varyings[4] = {"type1", "position1", "velocity1", "age1"}; 
    glTransformFeedbackVaryings(glusProgram.program, 4, Varyings, GL_INTERLEAVED_ATTRIBS);
    glusProgramLink(&glusProgram);
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    particleUpdateProgram.loadUniforms(glusProgram.program);
}

GLUSboolean init(GLUSvoid)
{
    initLightingProgram();
    initParticleUpdateProgram();
    initBillbardProgram();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    pGroundMesh = new Mesh();
    pGroundMesh->LoadMesh("quad.obj");
    pTexture = new Texture(GL_TEXTURE_2D, "../content/bricks.jpg");
    pNormalMap = new Texture(GL_TEXTURE_2D, "../content/normal_map.jpg");
    pRandomTexture = new RandomTexture(RANDOM_TEXTURE_SIZE);

    camera.setPosition(0.0f, 0.4f, -0.5f);
    camera.setDirection(0.0f, 0.2f, 1.0f);
    camera.setUpDireciton(0.0, 1.0f, 0.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    directionalLight.color = {1.0f, 1.0f, 1.0f};
    directionalLight.ambientIntensity = 0.2f;
    directionalLight.diffuseIntensity = 0.8f;

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    glViewport(0, 0, width, height);
}

void renderGround()
{
    glUseProgram(lightingProgram.program);

    // model view
    Matrix4x4f modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.rotatex(90.0f);
    modelViewMatrix = modelViewMatrix.scale(Vector3f{20.0f, 20.0f, 1.0f});
    Matrix4x4f viewMatrix = camera.getMatrix();
    modelViewMatrix = viewMatrix * modelViewMatrix;
    glUniformMatrix4fv(lightingProgram.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // model view for normal
    modelViewMatrix = modelViewMatrix.inverse().transpose();
    glUniformMatrix4fv(lightingProgram.modelViewMatrixForNormal, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // projection
    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(lightingProgram.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    // lighing
    directionalLight.direction = viewMatrix * Vector3f{1.0f, 0.0f, 0.0f};
    lightingProgram.setDirectionalLight(directionalLight);
    lightingProgram.setPointLights(nullptr, 0);
    lightingProgram.setSpotLights(nullptr, 0); 
    glUniform1f(lightingProgram.specular.specularIntensity, 0.0f);
    glUniform1f(lightingProgram.specular.shiness, 0.0f);

    // texture
    glUniform1i(lightingProgram.textureSampler, 0);
    glUniform1i(lightingProgram.normalSampler, 2);
    pTexture->Bind(GL_TEXTURE0);
    pNormalMap->Bind(GL_TEXTURE2);

    pGroundMesh->Render();

    glUseProgram(0);
}

void updateParticles(GLUSfloat deltaTime)
{
    deltaTime *= 1000.0f;
    static GLUSfloat time = 0.0f;
    time += deltaTime;

    glUseProgram(particleUpdateProgram.program);

    particleUpdateProgram.setTime(deltaTime, time);
    particleUpdateProgram.setLifetime(100.0f, 10000.0f, 2500.0f);

    glUniform1i(particleUpdateProgram.randomTextureSampler, 1);
    pRandomTexture->Bind(GL_TEXTURE1);

    glEnable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[currVB]); 
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbacks[currTFB]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0); // type
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4); // position
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16); // velocity
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28); // lifetime

    glBeginTransformFeedback(GL_POINTS);
    if (isfirst) {
        glDrawArrays(GL_POINTS, 0, 1);
        isfirst = false;
    } else {
        glDrawTransformFeedback(GL_POINTS, transformFeedbacks[currVB]);
    } 
    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glUseProgram(0);
}

void renderParticles()
{
    glUseProgram(billboardProgram.program);

    glDisable(GL_RASTERIZER_DISCARD);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[currTFB]);

    glUniformMatrix4fv(billboardProgram.modelViewMatrix, 1, GL_FALSE, camera.getMatrix().const_value_ptr());
    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(billboardProgram.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4); // position
    glDrawTransformFeedback(GL_POINTS, transformFeedbacks[currTFB]);
    glDisableVertexAttribArray(0);

    glUseProgram(0);
}

GLUSboolean update(GLUSfloat deltaTime)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderGround();
    updateParticles(deltaTime);
    renderParticles();

    currVB = 1 - currVB;
    currTFB = 1 - currTFB;

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
    delete pGroundMesh;
    delete pTexture;
    delete pNormalMap;
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

