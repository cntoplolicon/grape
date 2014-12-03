#include <iostream>
#include <cassert>
#include "GL/glus.h"
#include "camera.hpp"
#include "texture.hpp"

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1200;

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;                                                      

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



struct BaseLight
{
    Vector3f color;
    GLfloat ambientIntensity;
    GLfloat diffuseIntensity;
};

struct DirectionalLight
{
    BaseLight base;
    Vector3f direction;
};

struct Attenuation
{
    GLfloat constant = 1.0f;
    GLfloat linear = 0.0f;
    GLfloat quadratic = 0.0f;
};

struct PointLight
{
    BaseLight base;
    Vector3f position;
    Attenuation attenuation;
};

struct SpotLight
{
    PointLight base;
    Vector3f direction;
    float cutoff;
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

struct BaseLineUniform
{
    GLuint color;
    GLuint ambientIntensity;
    GLuint diffuseIntensity;
};

struct DirectionalLightUnform
{
    BaseLineUniform base;
    GLuint direction;
};

struct AttenuationUniform
{
    GLuint constant;
    GLuint linear;
    GLuint quadratic;
};

struct PointLightUniform
{
    BaseLineUniform base;
    GLuint position;
    AttenuationUniform attenuation;
};

struct SpotLightUniform
{
    PointLightUniform base;
    GLuint direction;
    GLuint cutoff;
};

struct Program
{
    GLuint program;
    GLuint position;
    GLuint texCoord;
    GLuint normal;
    GLuint modelViewMatrix;
    GLuint modelViewMatrixForNormal;
    GLuint projectionMatrix;
    GLuint textureSampler;

    DirectionalLightUnform directionalLight;
    GLuint numPointLights;
    PointLightUniform pointLights[MAX_POINT_LIGHTS];
    GLuint numSpotLights;
    SpotLightUniform spotLights[MAX_SPOT_LIGHTS];

    struct
    {
        GLuint specularIntensity;
        GLuint shiness;
    } material;
};

Program program;
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

    program.program = glusProgram.program;
    program.position = glGetAttribLocation(program.program, "position");
    program.texCoord = glGetAttribLocation(program.program, "texCoord");
    program.normal = glGetAttribLocation(program.program, "normal");

    program.modelViewMatrix = glGetUniformLocation(program.program, "modelViewMatrix");
    program.modelViewMatrixForNormal = glGetUniformLocation(program.program, "modelViewMatrixForNormal");
    program.projectionMatrix = glGetUniformLocation(program.program, "projectionMatrix");

    program.textureSampler = glGetUniformLocation(program.program, "textureSampler");

    program.material.specularIntensity = glGetUniformLocation(program.program, "specularIntensity");
    program.material.shiness = glGetUniformLocation(program.program, "shiness");

    program.directionalLight.base.color = glGetUniformLocation(program.program, "directionalLight.base.color");
    program.directionalLight.base.ambientIntensity = glGetUniformLocation(program.program, "directionalLight.base.ambientIntensity");
    program.directionalLight.base.diffuseIntensity = glGetUniformLocation(program.program, "directionalLight.base.diffuseIntensity");
    program.directionalLight.direction = glGetUniformLocation(program.program, "directionalLight.direction");

    program.numPointLights = glGetUniformLocation(program.program, "numPointLights");
    for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
        char buffer[256];
        sprintf(buffer, "pointLights[%d].base.color", i);
        program.pointLights[i].base.color = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "pointLights[%d].base.ambientIntensity", i);
        program.pointLights[i].base.ambientIntensity = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "pointLights[%d].base.diffuseIntensity", i);
        program.pointLights[i].base.diffuseIntensity = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "pointLights[%d].position", i);
        program.pointLights[i].position = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "pointLights[%d].attenuation.constant", i);
        program.pointLights[i].attenuation.constant = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "pointLights[%d].attenuation.linear", i);
        program.pointLights[i].attenuation.linear = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "pointLights[%d].attenuation.quadratic", i);
        program.pointLights[i].attenuation.quadratic= glGetUniformLocation(program.program, buffer);
    }
    program.numSpotLights = glGetUniformLocation(program.program, "numSpotLights");
    assert(program.numSpotLights != 0xffffffff);
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
        char buffer[256];
        sprintf(buffer, "spotLights[%d].base.base.color", i);
        program.spotLights[i].base.base.color = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].base.base.ambientIntensity", i);
        program.spotLights[i].base.base.ambientIntensity = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].base.base.diffuseIntensity", i);
        program.spotLights[i].base.base.diffuseIntensity = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].base.position", i);
        program.spotLights[i].base.position = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].base.attenuation.constant", i);
        program.spotLights[i].base.attenuation.constant = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].base.attenuation.linear", i);
        program.spotLights[i].base.attenuation.linear = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].base.attenuation.quadratic", i);
        program.spotLights[i].base.attenuation.quadratic = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].direction", i);
        program.spotLights[i].direction = glGetUniformLocation(program.program, buffer);
        sprintf(buffer, "spotLights[%d].cutoff", i);
        program.spotLights[i].cutoff = glGetUniformLocation(program.program, buffer);
    }
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
    camera.SetPosition(5.0f, 1.0f, -3.0f);
    camera.SetDirection(0.0f, 0.0f, 1.0f);
    camera.SetUpDireciton(0.0, 1.0f, 0.0f);

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
    GLfloat modelMatrix[16];
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, 1.0f);
    GLfloat viewMatrix[16];
    camera.GetMatrix(viewMatrix);
    GLfloat modelViewMatrix[16];
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glUniformMatrix4fv(program.modelViewMatrix, 1, GL_FALSE, modelViewMatrix);

    // model view for normal
    glusMatrix4x4Inversef(modelViewMatrix);
    glusMatrix4x4Transposef(modelViewMatrix);
    glUniformMatrix4fv(program.modelViewMatrixForNormal, 1, GL_FALSE, modelViewMatrix);

    // projection
    GLfloat projectionMatrix[16];
    glusMatrix4x4Perspectivef(projectionMatrix, 60.0f, (GLUSfloat)WINDOW_WIDTH / (GLUSfloat)WINDOW_HEIGHT, 1.0f, 50.0f);
    glUniformMatrix4fv(program.projectionMatrix, 1, GL_FALSE, projectionMatrix);

    // directional light
    glUniform3fv(program.directionalLight.base.color, 1, directionalLight.base.color.value_ptr());
    glUniform1f(program.directionalLight.base.ambientIntensity, directionalLight.base.ambientIntensity);
    glUniform1f(program.directionalLight.base.diffuseIntensity, directionalLight.base.diffuseIntensity);
    glUniform3fv(program.directionalLight.direction, 1, directionalLight.direction.value_ptr());

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
    glUniform1i(program.numPointLights, MAX_POINT_LIGHTS);
    for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
        glUniform3fv(program.pointLights[i].base.color, 1, pointLights[i].base.color.value_ptr());
        glUniform1f(program.pointLights[i].base.ambientIntensity, pointLights[i].base.ambientIntensity);
        glUniform1f(program.pointLights[i].base.diffuseIntensity, pointLights[i].base.diffuseIntensity);
        glUniform3fv(program.pointLights[i].position, 1, pointLights[i].position.value_ptr());
        glUniform1f(program.pointLights[i].attenuation.constant, pointLights[i].attenuation.constant);
        glUniform1f(program.pointLights[i].attenuation.linear, pointLights[i].attenuation.linear);
        glUniform1f(program.pointLights[i].attenuation.quadratic, pointLights[i].attenuation.quadratic);
    }

    // spot light
    spotLights[0].base.base.diffuseIntensity = 0.9f;
    spotLights[0].base.base.color = {0.0f, 1.0f, 1.0f};
    spotLights[0].base.position = camera.GetPosition();
    spotLights[0].direction = camera.GetDirection();
    spotLights[0].base.attenuation.linear = 0.1f;
    spotLights[0].cutoff = 10.0f;
    spotLights[1].base.base.diffuseIntensity = 0.9f;
    spotLights[1].base.base.color = {1.0f, 1.0f, 1.0f};
    spotLights[1].base.position = {5.0f, 3.0f, 10.0f};
    spotLights[1].direction = {0.0f, -1.0f, 0.0f};
    spotLights[1].base.attenuation.linear = 0.1f;
    spotLights[1].cutoff = 20.0f;
    glUniform1i(program.numSpotLights, 2);
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
        glUniform3fv(program.spotLights[i].base.base.color, 1, spotLights[i].base.base.color.value_ptr());
        glUniform1f(program.spotLights[i].base.base.ambientIntensity, spotLights[i].base.base.ambientIntensity);
        glUniform1f(program.spotLights[i].base.base.diffuseIntensity, spotLights[i].base.base.diffuseIntensity);
        glUniform3fv(program.spotLights[i].base.position, 1, spotLights[i].base.position.value_ptr());
        glUniform1f(program.spotLights[i].base.attenuation.constant, spotLights[i].base.attenuation.constant);
        glUniform1f(program.spotLights[i].base.attenuation.linear, spotLights[i].base.attenuation.linear);
        glUniform1f(program.spotLights[i].base.attenuation.quadratic, spotLights[i].base.attenuation.quadratic);
        glUniform3fv(program.spotLights[i].direction, 1, spotLights[i].direction.value_ptr());
        glUniform1f(program.spotLights[i].cutoff, spotLights[i].cutoff);
    }

    // specular lighting
    glUniform1f(program.material.specularIntensity, 0.0f);
    glUniform1f(program.material.shiness, 0.0f);

    glUniform1i(program.textureSampler, 0);
    pTexture->Bind(GL_TEXTURE0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);

    return GLUS_TRUE;
}

GLUSvoid keyboard(GLUSboolean pressed, GLUSint key)
{
    camera.OnKey(pressed, key);
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
        default:
            break;
    }
    directionalLight.base.ambientIntensity = glusMathClampf(directionalLight.base.ambientIntensity, 0.0f, 1.0f);
    directionalLight.base.diffuseIntensity = glusMathClampf(directionalLight.base.diffuseIntensity, 0.0f, 1.0f);
}

GLUSvoid mouseMove(GLUSint buttons, GLUSint x, GLUSint y)
{
    camera.OnMouse(buttons, x, y);
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

