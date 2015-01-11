#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "lighting_program.hpp"
#include "mesh.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 1024;

struct MVP
{
    Matrix4x4f modelMatrix;
    Matrix4x4f viewMatrix;
    Matrix4x4f modelViewMatrix;
    Matrix4x4f modelViewMatrixForNormal;
    Matrix4x4f projectionMatrix;
    Matrix4x4f modelViewProjMatrix;

    MVP(const Matrix4x4f &modelMatrix, const Matrix4x4f &viewMatrix, const Matrix4x4f &projectionMatrix)
    {
        this->modelMatrix = modelMatrix;
        this->viewMatrix = viewMatrix;
        this->projectionMatrix = projectionMatrix;
        modelViewMatrix = viewMatrix * modelMatrix;
        modelViewMatrixForNormal = modelViewMatrix.inverse().transpose();
        modelViewProjMatrix = projectionMatrix * modelViewMatrix;
    }
};

struct ShadowVolumnProgram
{
    GLuint program;

    GLuint MVPMatrix;
    GLuint lightPosition;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        MVPMatrix = glGetUniformLocation(program, "MVPMatrix");
        lightPosition = glGetUniformLocation(program, "lightPosition");
    }
};

LightingProgram<2, 2> lightingProgram;
ShadowVolumnProgram shadowProgram;
DirectionalLight directionalLight;
PointLight pointLight;
Camera camera;
Mesh *pBoxMesh;
Mesh *pQuadMesh;
Texture *pQuadTexture;

void initLightingProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("../framework/lighting.vs.glsl", &vertexSource);
    glusFileLoadText("../framework/lighting.fs.glsl", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    lightingProgram.loadUniforms(glusProgram.program);
}

void initShadowProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./shadow.vs.glsl", &vertexSource);
    glusFileLoadText("./shadow.gs.glsl", &geometrySource);
    glusFileLoadText("./shadow.fs.glsl", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, const_cast<const GLUSchar **>(&geometrySource.text), 
            const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    shadowProgram.loadUniforms(glusProgram.program);
}

GLUSboolean init(GLUSvoid)
{
    initLightingProgram();
    initShadowProgram();

    pBoxMesh = new Mesh();
    pBoxMesh->LoadMesh("../content/box.obj", true);
    pQuadMesh = new Mesh();
    pQuadMesh->LoadMesh("../content/quad.obj", false);
    pQuadTexture = new Texture(GL_TEXTURE_2D, "../content/test.png");

    camera.setPosition(0.0f, 20.0f, -7.0f);
    camera.setDirection(0.0f, -1.0f, 1.0f);
    camera.setUpDireciton(0.0, 1.0f, 0.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);                  

    directionalLight.color = {1.0f, 1.0f, 1.0f};
    directionalLight.ambientIntensity = 0.0f;
    directionalLight.diffuseIntensity = 0.0f;

    pointLight.color = {1.0f, 1.0f, 1.0f};
    pointLight.position = {0.0f, 10.0f, 0.0f};

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    glViewport(0, 0, width, height);
}

void renderScene(const MVP &boxMVP, const MVP &quadMVP)
{
    // model view
    glUniformMatrix4fv(lightingProgram.modelViewMatrix, 1, GL_FALSE, boxMVP.modelViewMatrix.const_value_ptr());
    glUniformMatrix4fv(lightingProgram.viewMatrix, 1, GL_FALSE, boxMVP.viewMatrix.const_value_ptr());

    // model view for normal
    glUniformMatrix4fv(lightingProgram.modelViewMatrixForNormal, 1, GL_FALSE, boxMVP.modelViewMatrixForNormal.const_value_ptr());

    // projection
    glUniformMatrix4fv(lightingProgram.projectionMatrix, 1, GL_FALSE, boxMVP.projectionMatrix.const_value_ptr());

    // lighting
    directionalLight.direction = boxMVP.viewMatrix * Vector3f{1.0f, 0.0f, 0.0f};
    lightingProgram.setDirectionalLight(directionalLight);
    lightingProgram.setPointLights(&pointLight, 1);
    lightingProgram.setSpotLights(nullptr, 0); 
    glUniform1f(lightingProgram.specular.specularIntensity, 0.0f);
    glUniform1f(lightingProgram.specular.shiness, 0.0f);

    pBoxMesh->Render();

    // quad

    // model view
    glUniformMatrix4fv(lightingProgram.modelViewMatrix, 1, GL_FALSE, quadMVP.modelViewMatrix.const_value_ptr());

    // model view for normal
    glUniformMatrix4fv(lightingProgram.modelViewMatrixForNormal, 1, GL_FALSE, quadMVP.modelViewMatrixForNormal.const_value_ptr());

    // projection
    glUniformMatrix4fv(lightingProgram.projectionMatrix, 1, GL_FALSE, quadMVP.projectionMatrix.const_value_ptr());

    // texture
    glUniform1i(lightingProgram.textureSampler, 0);
    pQuadTexture->Bind(GL_TEXTURE0);

    pQuadMesh->Render();
}

void renderShadowInfoStencil(const MVP &boxMVP)
{
    glDrawBuffer(GL_NONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_CLAMP);        
    glDisable(GL_CULL_FACE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);       

    glUseProgram(shadowProgram.program);

    glUniformMatrix4fv(shadowProgram.MVPMatrix, 1, GL_FALSE,  boxMVP.modelViewProjMatrix.const_value_ptr());
    glUniform3fv(shadowProgram.lightPosition, 1, pointLight.position.const_value_ptr());

    pBoxMesh->Render();

    glUseProgram(0);

    glDisable(GL_DEPTH_CLAMP);
    glEnable(GL_CULL_FACE);                  
    glDepthMask(GL_TRUE);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDrawBuffer(GL_BACK);
}

void renderShadowedScene(const MVP &boxMVP, const MVP &quadMVP)
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glStencilFunc(GL_EQUAL, 0x0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(lightingProgram.program);

    pointLight.ambientIntensity = 0.0f;
    pointLight.diffuseIntensity = 0.8f;

    renderScene(boxMVP, quadMVP);

    glUseProgram(0);
    glDisable(GL_BLEND);
}

void renderAmbientScene(const MVP &boxMVP, const MVP &quadMVP)
{
    glUseProgram(lightingProgram.program);

    pointLight.ambientIntensity = 0.2f;
    pointLight.diffuseIntensity = 0.0f;

    renderScene(boxMVP, quadMVP); 

    glUseProgram(0);
}

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    Matrix4x4f m = Matrix4x4f::identity().translate({1.0f, 2.0f, 0.0f});
    Matrix4x4f v = camera.getMatrix();
    Matrix4x4f p = Matrix4x4f::perspective(60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 1000.0f);
    MVP boxMVP(m, v, p);

    m = Matrix4x4f::identity().rotatex(90.0f).scale({10.0f, 10.0f, 10.0f});
    MVP quadMVP(m, v, p);

    renderAmbientScene(boxMVP, quadMVP);
    glEnable(GL_STENCIL_TEST);
    renderShadowInfoStencil(boxMVP);
    renderShadowedScene(boxMVP, quadMVP);
    glDisable(GL_STENCIL_TEST);

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
    delete pBoxMesh;
}

int main(int argc, char* argv[])
{
    MagickCore::MagickCoreGenesis(*argv, Magick::MagickFalse);
    EGLint eglConfigAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
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

