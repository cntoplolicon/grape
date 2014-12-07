#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "lighting_program.hpp"

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1200;

DirectionalLight directionalLight;
Mesh *pMesh = nullptr;
LightingProgram<0, 0> program;
Camera camera;

Mesh *sphereMesh;
GLuint textureObject;
const char *texture_files[] = {
    "../content/sp3right.jpg",
    "../content/sp3left.jpg",
    "../content/sp3top.jpg",
    "../content/sp3bot.jpg",
    "../content/sp3front.jpg",
    "../content/sp3back.jpg"
};

struct SkyboxProgram
{
    GLuint program;
    GLuint position;
    GLuint modelViewMatrix;
    GLuint projectionMatrix;
    GLuint cubemapTexture;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        position = glGetAttribLocation(program, "position");
        modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
        cubemapTexture = glGetUniformLocation(program, "cubemapTexture");
    }

} skyboxProgram;

void initProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram glusProgram;

    glusFileLoadText("./lighting.vs", &vertexSource);
    glusFileLoadText("./lighting.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    program.loadUniforms(glusProgram.program);

    glusFileLoadText("./skybox.vs", &vertexSource);
    glusFileLoadText("./skybox.fs", &fragmentSource);
    glusProgramBuildFromSource(&glusProgram, const_cast<const GLUSchar **>(&vertexSource.text),
            0, 0, 0, const_cast<const GLUSchar **>(&fragmentSource.text));
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    skyboxProgram.loadUniforms(glusProgram.program);
}

void initCubeMap()
{
    const GLenum types[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };


    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject);

    Magick::Blob blob;

    for (int i = 0 ; i < 6 ; i++) {
        try {            
            Magick::Image image(texture_files[i]);
            image.write(&blob, "RGBA");
            glTexImage2D(types[i], 0, GL_RGB, image.columns(), image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, blob.data());
        }
        catch (Magick::Error& error) {
            std::cerr << "Error loading texture '" << texture_files[i] << "': " << error.what() << std::endl;
            throw;
        }
    }    
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

GLUSboolean init(GLUSvoid)
{
    initProgram();
    initCubeMap();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    pMesh = new Mesh();
    pMesh->LoadMesh("../content/phoenix_ugv.md2");
    sphereMesh = new Mesh();
    sphereMesh->LoadMesh("../content/sphere.obj");

    camera.setPosition(0.0f, 1.0f, -20.0f);
    camera.setDirection(0.0f, 0.0f, 1.0f);
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

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program.program);

    // model view
    static float m_scale = 0.0f;
    m_scale += 0.05f;
    Matrix4x4f modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.translate({0.0f, -5.0f, 3.0f});
    modelViewMatrix = modelViewMatrix.rotatey(m_scale);
    modelViewMatrix = modelViewMatrix.scale({0.1f, 0.1f, 0.1f});
    Matrix4x4f viewMatrix = camera.getMatrix();
    modelViewMatrix = viewMatrix * modelViewMatrix;
    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // model view for normal
    modelViewMatrix = modelViewMatrix.inverse().transpose();
    glUniformMatrix4fv(program.modelViewMatrixForNormal, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // projection
    Matrix4x4f projectionMatrix = Matrix4x4f::perspective(60.0f, WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 100.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    // directional light
    directionalLight.direction = viewMatrix * Vector3f{1.0f, -1.0f, 0.0f};
    program.setDirectionalLight(directionalLight);

    // point light
    program.setPointLights(nullptr, 0);

    // spot light
    program.setSpotLights(nullptr, 0); 

    // specular lighting
    glUniform1f(program.specular.specularIntensity, 0.0f);
    glUniform1f(program.specular.shiness, 0.0f);

    // texture
    glUniform1i(program.textureSampler, 0);

    pMesh->Render();

    // skybox
    glUseProgram(skyboxProgram.program);

    // depth function
    GLint oldDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
    glDepthFunc(GL_LEQUAL);

    // modelview
    modelViewMatrix = Matrix4x4f::identity();
    modelViewMatrix = modelViewMatrix.translate(camera.getPosition());
    modelViewMatrix = modelViewMatrix.scale({20.0f, 20.0f, 20.0f});
    modelViewMatrix = viewMatrix * modelViewMatrix;
    glUniformMatrix4fv(skyboxProgram.modelViewMatrix, 1, GL_FALSE, modelViewMatrix.const_value_ptr());

    // projection
    glUniformMatrix4fv(skyboxProgram.projectionMatrix, 1, GL_FALSE, projectionMatrix.const_value_ptr());

    // cubemap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject);

    sphereMesh->Render();  
    
    // restore depth function
    glDepthFunc(oldDepthFunc);

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

