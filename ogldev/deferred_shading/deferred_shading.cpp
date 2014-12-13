#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "gbuffer.hpp"
#include "lighting_program.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 1024;

const Vector3f COLOR_RED = {1.0f, 0.0f, 0.0f};
const Vector3f COLOR_GREEN = {0.0f, 1.0f, 0.0f};
const Vector3f COLOR_CYAN = {0.0f, 1.0f, 1.0f};
const Vector3f COLOR_BLUE = {0.0f, 0.0f, 1.0f};

Mesh *pBoxMesh;
Mesh *pQuadMesh;
Mesh *pSphereMesh;
GBuffer *pGBuffer;
Camera camera;

struct MVPPipeline
{
    GLuint program;
    GLuint modelMatrix;
    GLuint viewMatrix;
    GLuint projectionMatrix;

    virtual void loadUniforms(GLuint program)
    {
        this->program = program;
        modelMatrix = glGetUniformLocation(program, "modelMatrix");
        viewMatrix = glGetUniformLocation(program, "viewMatrix");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
    }
};

struct GeometryPass : public MVPPipeline
{
    GLuint textureSampler;

    virtual void loadUniforms(GLuint program)
    {
        MVPPipeline::loadUniforms(program);
        textureSampler = glGetUniformLocation(program, "textureSampler");
    }
};

struct LightPass : public MVPPipeline
{
    GLuint positionSampler;
    GLuint colorSampler;
    GLuint normalSampler;
    GLuint cameraMatrix;
    GLuint screenSize;
    GLuint specularIntensity;
    GLuint shiness;

    virtual void loadUniforms(GLuint program)
    {
        MVPPipeline::loadUniforms(program);
        positionSampler = glGetUniformLocation(program, "positionSampler");
        colorSampler = glGetUniformLocation(program, "colorSampler");
        normalSampler = glGetUniformLocation(program, "normalSampler");
        cameraMatrix = glGetUniformLocation(program, "cameraMatrix");
        screenSize = glGetUniformLocation(program, "screenSize");
        specularIntensity = glGetUniformLocation(program, "specularIntensity");
        shiness = glGetUniformLocation(program, "shiness");
    }

    void bindUniforms()
    {
        glUniform1i(positionSampler, GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
        glUniform1i(colorSampler, GBuffer::GBUFFER_TEXTURE_TYPE_COLOR);
        glUniform1i(normalSampler, GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        glUniformMatrix4fv(cameraMatrix, 1, GL_FALSE, camera.getMatrix().const_value_ptr());
        glUniform1f(specularIntensity, 0.0f);
        glUniform1f(shiness, 0.0f);

        GLfloat screen[] = {WINDOW_WIDTH, WINDOW_HEIGHT};
        glUniform2fv(screenSize, 1, screen);
    }
};

struct DirectionalLightPass : public LightPass
{
    DirectionalLightUnform directionalLight;

    virtual void loadUniforms(GLuint program)
    {
        LightPass::loadUniforms(program);
        directionalLight.color = glGetUniformLocation(program, "directionalLight.base.color");
        directionalLight.ambientIntensity = glGetUniformLocation(program, "directionalLight.base.ambientIntensity");
        directionalLight.diffuseIntensity = glGetUniformLocation(program, "directionalLight.base.diffuseIntensity");
        directionalLight.direction = glGetUniformLocation(program, "directionalLight.direction");
    }
};

struct PointLightPass : public LightPass
{
    PointLightUniform pointLight;

    void loadUniforms(GLuint program)
    {
        LightPass::loadUniforms(program);
        pointLight.color = glGetUniformLocation(program, "pointLight.base.color");
        pointLight.ambientIntensity = glGetUniformLocation(program, "pointLight.base.ambientIntensity");
        pointLight.diffuseIntensity = glGetUniformLocation(program, "pointLight.base.diffuseIntensity");
        pointLight.position = glGetUniformLocation(program, "pointLight.position");
        pointLight.attenuation.constant = glGetUniformLocation(program, "pointLight.attenuation.constant");
        pointLight.attenuation.linear = glGetUniformLocation(program, "pointLight.attenuation.linear");
        pointLight.attenuation.quadratic= glGetUniformLocation(program, "pointLight.attenuation.quadratic");
    }
};

struct StencilPass : public MVPPipeline
{
    void loadUniforms(GLuint program)
    {
        MVPPipeline::loadUniforms(program);
    }
};

GeometryPass geometryPass;
DirectionalLightPass directionalLightPass;
StencilPass stencilPass;
PointLightPass pointLightPass;

Vector3f boxPositions[5];
PointLight pointLights[3];
DirectionalLight directionalLight;

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
    directionalLight.ambientIntensity = 0.1f;
    directionalLight.color = COLOR_CYAN;
    directionalLight.diffuseIntensity = 0.5f;
    directionalLight.direction = {1.0f, 0.0f, 0.0f};

    pointLights[0].diffuseIntensity = 0.2f;
    pointLights[0].color = COLOR_GREEN;
    pointLights[0].position = {0.0f, 1.5f, 5.0f};
    pointLights[0].attenuation.constant = 1.0f;
    pointLights[0].attenuation.linear = 0.0f;
    pointLights[0].attenuation.quadratic = 0.3f;

    pointLights[1].diffuseIntensity = 0.2f;
    pointLights[1].color = COLOR_RED;
    pointLights[1].position = {2.0f, 0.0f, 5.0f};
    pointLights[1].attenuation.constant = 1.0f;
    pointLights[1].attenuation.linear = 0.0f;
    pointLights[1].attenuation.quadratic = 0.3f;

    pointLights[2].diffuseIntensity = 0.2f;
    pointLights[2].color = COLOR_BLUE;
    pointLights[2].position = {0.0f, 0.0f, 3.0f};
    pointLights[2].attenuation.constant = 1.0f;
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

void initDirectionalLightPass()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./light_pass.vs", &vertexSource);
    glusFileLoadText("./directional_light_pass.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    directionalLightPass.loadUniforms(glusProgram.program);
}

void initStencilPass()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./light_pass.vs", &vertexSource);
    glusFileLoadText("./empty.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    stencilPass.loadUniforms(glusProgram.program);
}

void initPointLightPass()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./light_pass.vs", &vertexSource);
    glusFileLoadText("./point_light_pass.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    pointLightPass.loadUniforms(glusProgram.program);
}

GLUSboolean init(GLUSvoid)
{
    initBoxPositions();
    initLights();
    initGeometryPass();
    initDirectionalLightPass();
    initStencilPass();
    initPointLightPass();

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
    glEnable(GL_STENCIL_TEST);

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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    pGBuffer->bindForWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(geometryPass.program);

    Matrix4x4f viewMatrix = camera.getMatrix();
    glUniformMatrix4fv(geometryPass.viewMatrix, 1, GL_FALSE, viewMatrix.const_value_ptr());

    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(geometryPass.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    glUniform1i(geometryPass.textureSampler, 0);
    for (size_t i = 0; i < sizeof(boxPositions) / sizeof(boxPositions[0]); i++) {
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

    pGBuffer->bindForReading();
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderDirectionalLightPass()
{
    glCullFace(GL_FRONT);
    glUseProgram(directionalLightPass.program);

    Matrix4x4f identity = Matrix4x4f::identity();
    glUniformMatrix4fv(directionalLightPass.modelMatrix, 1, GL_FALSE, identity.const_value_ptr());
    glUniformMatrix4fv(directionalLightPass.viewMatrix, 1, GL_FALSE, identity.const_value_ptr());
    glUniformMatrix4fv(directionalLightPass.projectionMatrix, 1, GL_FALSE, identity.const_value_ptr());

    directionalLightPass.bindUniforms();
    directionalLightPass.directionalLight.setDirectionalLight(directionalLight);

    pQuadMesh->Render();

    glUseProgram(0);
}

float calcPointLightScale(const PointLight& light)
{
    float maxChannel = fmax(fmax(light.color.x, light.color.y), light.color.z);

    float ret = (-light.attenuation.linear + sqrtf(light.attenuation.linear * light.attenuation.linear -
                4 * light.attenuation.quadratic * (light.attenuation.constant - 256 * maxChannel * light.diffuseIntensity)))
        /
        2 * light.attenuation.quadratic;

    return ret;
}

void renderLightVolume(const MVPPipeline &program, const PointLight &light)
{
    Matrix4x4f modelMatrix = Matrix4x4f::identity();
    modelMatrix = modelMatrix.translate(light.position);
    float scale = calcPointLightScale(light);
    modelMatrix = modelMatrix.scale({scale, scale, scale});
    glUniformMatrix4fv(program.modelMatrix, 1, GL_FALSE, modelMatrix.const_value_ptr());

    Matrix4x4f viewMatrix = camera.getMatrix();
    glUniformMatrix4fv(program.viewMatrix, 1, GL_FALSE, viewMatrix.const_value_ptr());

    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    pSphereMesh->Render();
}

void renderStencilPass(const PointLight &light)
{
    glClear(GL_STENCIL_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glEnable(GL_DEPTH_TEST);

    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

    glUseProgram(stencilPass.program);

    renderLightVolume(stencilPass, light);

    glUseProgram(0);

    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

void renderPointLightPass(const PointLight &light)
{
    glStencilFunc(GL_NOTEQUAL, 0, 0xff);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glUseProgram(pointLightPass.program);

    pointLightPass.bindUniforms();
    pointLightPass.pointLight.setPointLight(light);

    renderLightVolume(pointLightPass, light);

    glUseProgram(0);
}

void renderPointLights()
{
    for (size_t i = 0; i < sizeof(pointLights) / sizeof(pointLights[0]); i++) {
        renderStencilPass(pointLights[i]);
        renderPointLightPass(pointLights[i]);        
    }
}

GLUSboolean update(GLUSfloat time)
{
    renderGeometryPass(); 
    beginLightPasses();
    renderDirectionalLightPass();
    renderPointLights();

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
        EGL_STENCIL_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint eglContextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
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

