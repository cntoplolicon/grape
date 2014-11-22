#include "GL/glus.h"
#include "glutil/glutil.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "mesh.hpp"
#include "mouse_pole.hpp"
#include "timer.hpp"

struct ProgramData
{
    GLuint program;

    GLuint modelViewUnif;
    GLuint modelViewForNormalUnif;

    GLuint lightIntensityUnif;
    GLuint ambientIntensityUnif;
    GLuint cameraSpaceLightPosUnif;
    GLuint lightAttenuationUnif;
    GLuint shininessFactorUnif;
    GLuint baseDiffuseColorUnif;
};

struct UnlitProgData
{
    GLuint program;
    GLuint objectColorUnif;
    GLuint modelViewUnif;
};

float g_fzNear = 1.0f;
float g_fzFar = 1000.0f;

ProgramData g_MonoDiffuseProgram;
ProgramData g_ColorDiffuseProgram;
UnlitProgData g_UnlitProgram;

const int g_projectionBlockIndex = 2;

GLuint loadProgram(const std::string &vertShaderFile, const std::string &fragShaderFile)
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram program;

    glusFileLoadText(vertShaderFile.c_str(), &vertexSource);
    glusFileLoadText(fragShaderFile.c_str(), &fragmentSource);
    glusProgramBuildFromSource(&program, const_cast<const GLUSchar**>(&vertexSource.text), 0, 0, 0, const_cast<const GLUSchar**>(&fragmentSource.text));

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    return program.program;
}

UnlitProgData loadUnlitProgram(const std::string &vertShaderFile, const std::string &fragShaderFile)
{ 
    UnlitProgData data;
    data.program = loadProgram(vertShaderFile, fragShaderFile);
    data.modelViewUnif = glGetUniformLocation(data.program, "modelViewMatrix");
    data.objectColorUnif = glGetUniformLocation(data.program, "objectColor");

    GLuint projectionBlock = glGetUniformBlockIndex(data.program, "Projection");
    glUniformBlockBinding(data.program, projectionBlock, g_projectionBlockIndex);

    return data;
}

ProgramData loadLitProgram(const std::string &vertShaderFile, const std::string &fragShaderFile)
{
    ProgramData data;
    data.program = loadProgram(vertShaderFile, fragShaderFile);

    data.modelViewUnif = glGetUniformLocation(data.program, "modelViewMatrix");
    data.modelViewForNormalUnif = glGetUniformLocation(data.program, "modelViewMatrixForNormal");
    data.cameraSpaceLightPosUnif = glGetUniformLocation(data.program, "cameraSpaceLightPos");
    data.lightIntensityUnif = glGetUniformLocation(data.program, "lightIntensity");
    data.ambientIntensityUnif = glGetUniformLocation(data.program, "ambientIntensity");
    data.lightAttenuationUnif = glGetUniformLocation(data.program, "lightAttenuation");
    data.baseDiffuseColorUnif = glGetUniformLocation(data.program, "baseDiffuseColor");
    data.shininessFactorUnif = glGetUniformLocation(data.program, "shininessFactor");

    GLuint projectionBlock = glGetUniformBlockIndex(data.program, "Projection");
    glUniformBlockBinding(data.program, projectionBlock, g_projectionBlockIndex);

    return data;
}

void initPrograms()
{
    g_UnlitProgram = loadUnlitProgram("./light.vert", "./light.frag");
    g_MonoDiffuseProgram = loadLitProgram("./specular_lighting_mono.vert", "./specular_lighting.frag");
    g_ColorDiffuseProgram = loadLitProgram("./specular_lighting_color.vert", "./specular_lighting.frag");
}

Mesh *g_pCylinderMesh;
Mesh *g_pPlaneMesh;
Mesh *g_pCubeMesh;

glutil::ViewData g_initialViewData =
{
    glm::vec3(0.0f, 0.5f, 0.0f),
    glm::fquat(0.92387953f, 0.3826834f, 0.0f, 0.0f),
    5.0f,
    0.0f
};

glutil::ViewScale g_viewScale =
{
    3.0f, 20.0f,
    1.5f, 0.5f,
    0.0f, 0.0f,
    90.0f/250.0f
};

glutil::ObjectData g_initialObjectData =
{
    glm::vec3(0.0f, 0.5f, 0.0f),
    glm::fquat(1.0f, 0.0f, 0.0f, 0.0f),
};

glutil::ViewPole g_viewPole = glutil::ViewPole(g_initialViewData,
        g_viewScale, glutil::MB_LEFT_BTN);
glutil::ObjectPole g_objtPole = glutil::ObjectPole(g_initialObjectData,
        90.0f/250.0f, glutil::MB_RIGHT_BTN, &g_viewPole);

void mouseButton(GLUSboolean pressed, GLUSint button, GLUSint x, GLUSint y)
{
    forwardMouseClickToMousePole(g_viewPole, pressed, button, x, y);
    forwardMouseClickToMousePole(g_objtPole, pressed, button, x, y);
}

void mouseMotion(GLUSint buttons, GLUSint x, GLUSint y)
{
    forwardMouseMotionToMousePole(g_viewPole, x, y);
    forwardMouseMotionToMousePole(g_objtPole, x, y);
}

void mouseWheel(GLUSint wheel, GLUSint ticks, GLUSint x, GLUSint y)
{
    forwardMouseWheelToMousePole(g_viewPole, wheel, ticks, x, y);
    forwardMouseWheelToMousePole(g_objtPole, wheel, ticks, x, y);
}

GLuint g_projectionUniformBuffer = 0;

struct ProjectionBlock
{
    glm::mat4 cameraToClipMatrix;
};

GLUSboolean init()
{
    initPrograms();

    g_pCylinderMesh = new Mesh("./model/UnitCylinder.xml");
    g_pPlaneMesh = new Mesh("./model/LargePlane.xml");
    g_pCubeMesh = new Mesh("./model/UnitCube.xml");

    glusWindowSetMouseFunc(mouseButton);
    glusWindowSetMouseMoveFunc(mouseMotion);
    glusWindowSetMouseWheelFunc(mouseWheel);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    const float depthZNear = 0.0f;
    const float depthZFar = 1.0f;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(depthZNear, depthZFar);
    glEnable(GL_DEPTH_CLAMP);

    glGenBuffers(1, &g_projectionUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionBlock), NULL, GL_DYNAMIC_DRAW);

    glBindBufferRange(GL_UNIFORM_BUFFER, g_projectionBlockIndex, g_projectionUniformBuffer,
            0, sizeof(ProjectionBlock));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return GLUS_TRUE;
}

static float g_fLightHeight = 1.5f;
static float g_fLightRadius = 1.0f;
Timer g_LightTimer(Timer::TT_LOOP, 5.0f);

glm::vec4 CalcLightPosition()
{
    float fCurrTimeThroughLoop = g_LightTimer.GetAlpha();

    glm::vec4 ret(0.0f, g_fLightHeight, 0.0f, 1.0f);

    ret.x = cosf(fCurrTimeThroughLoop * (3.14159f * 2.0f)) * g_fLightRadius;
    ret.z = sinf(fCurrTimeThroughLoop * (3.14159f * 2.0f)) * g_fLightRadius;

    return ret;
}

static bool g_bDrawColoredCyl = false;
static bool g_bDrawLightSource = false;
static bool g_bScaleCyl = false;
static bool g_bDrawDark = false;

const float g_fLightAttenuation = 1.2f;

const glm::vec4 g_darkColor(0.2f, 0.2f, 0.2f, 1.0f);
const glm::vec4 g_lightColor(1.0f);

float g_fGaussianRoughness = 0.5f;

GLUSboolean display(GLUSfloat time)
{
    g_LightTimer.Update(time);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glutil::MatrixStack modelMatrix;
    modelMatrix.SetMatrix(g_viewPole.CalcMatrix());

    const glm::vec4 &worldLightPos = CalcLightPosition();
    const glm::vec4 &lightPosCameraSpace = modelMatrix.Top() * worldLightPos;

    glUseProgram(g_MonoDiffuseProgram.program);
    glUniform4f(g_MonoDiffuseProgram.lightIntensityUnif, 0.8f, 0.8f, 0.8f, 1.0f);
    glUniform4f(g_MonoDiffuseProgram.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
    glUniform3fv(g_MonoDiffuseProgram.cameraSpaceLightPosUnif,1, glm::value_ptr(lightPosCameraSpace));
    glUniform1f(g_MonoDiffuseProgram.lightAttenuationUnif, g_fLightAttenuation);
    glUniform1f(g_MonoDiffuseProgram.shininessFactorUnif, g_fGaussianRoughness);
    glUniform4fv(g_MonoDiffuseProgram.baseDiffuseColorUnif, 1,
            g_bDrawDark ? glm::value_ptr(g_darkColor) : glm::value_ptr(g_lightColor));

    glUseProgram(g_ColorDiffuseProgram.program);
    glUniform4f(g_ColorDiffuseProgram.lightIntensityUnif, 0.8f, 0.8f, 0.8f, 1.0f);
    glUniform4f(g_ColorDiffuseProgram.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
    glUniform3fv(g_ColorDiffuseProgram.cameraSpaceLightPosUnif, 1, glm::value_ptr(lightPosCameraSpace));
    glUniform1f(g_ColorDiffuseProgram.lightAttenuationUnif, g_fLightAttenuation);
    glUniform1f(g_ColorDiffuseProgram.shininessFactorUnif, g_fGaussianRoughness);
    glUseProgram(0);

    {
        glutil::PushStack push(modelMatrix);

        {
            glutil::PushStack push(modelMatrix);

            glm::mat3 normMatrix(modelMatrix.Top());
            normMatrix = glm::transpose(glm::inverse(normMatrix));

            glUseProgram(g_MonoDiffuseProgram.program);
            glUniformMatrix4fv(g_MonoDiffuseProgram.modelViewUnif, 1, GL_FALSE,
                    glm::value_ptr(modelMatrix.Top()));

            glUniformMatrix3fv(g_MonoDiffuseProgram.modelViewForNormalUnif, 1, GL_FALSE,
                    glm::value_ptr(normMatrix));
            g_pPlaneMesh->render();
            glUseProgram(0);
        }

        {
            glutil::PushStack push(modelMatrix);

            modelMatrix.ApplyMatrix(g_objtPole.CalcMatrix());

            if (g_bScaleCyl) {
                modelMatrix.Scale(1.0f, 1.0f, 0.2f);
            }

            glm::mat3 normMatrix(modelMatrix.Top());
            normMatrix = glm::transpose(glm::inverse(normMatrix));

            ProgramData &prog = g_bDrawColoredCyl ? g_ColorDiffuseProgram : g_MonoDiffuseProgram;
            glUseProgram(prog.program);
            glUniformMatrix4fv(prog.modelViewUnif, 1, GL_FALSE,
                    glm::value_ptr(modelMatrix.Top()));
            glUniformMatrix3fv(prog.modelViewForNormalUnif, 1, GL_FALSE,
                    glm::value_ptr(normMatrix));

            g_pCylinderMesh->render(g_bDrawColoredCyl ? "lit-color" : "lit");

            glUseProgram(0);
        }

        if (g_bDrawLightSource)
        {
            glutil::PushStack push(modelMatrix);

            modelMatrix.Translate(glm::vec3(worldLightPos));
            modelMatrix.Scale(0.1f, 0.1f, 0.1f);

            glUseProgram(g_UnlitProgram.program);
            glUniformMatrix4fv(g_UnlitProgram.modelViewUnif, 1, GL_FALSE,
                    glm::value_ptr(modelMatrix.Top()));
            glUniform4f(g_UnlitProgram.objectColorUnif, 0.8078f, 0.8706f, 0.9922f, 1.0f);
            g_pCubeMesh->render("flat");
        }
    }

    return GLUS_TRUE;
}

void reshape(int w, int h)
{
    glutil::MatrixStack persMatrix;
    persMatrix.Perspective(45.0f, (w / (float)h), g_fzNear, g_fzFar);

    ProjectionBlock projData;
    projData.cameraToClipMatrix = persMatrix.Top();

    glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ProjectionBlock), &projData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glViewport(0, 0, w, h);
}

void keyboard(const GLUSboolean pressed, const GLUSint key)
{
    if (!pressed) {
        return;
    }

    switch (key)
    {
        case 32:
            g_bDrawColoredCyl = !g_bDrawColoredCyl;
            break;

        case 'i': g_fLightHeight += 0.2f; break;
        case 'k': g_fLightHeight -= 0.2f; break;
        case 'l': g_fLightRadius += 0.2f; break;
        case 'j': g_fLightRadius -= 0.2f; break;
        case 'I': g_fLightHeight += 0.05f; break;
        case 'K': g_fLightHeight -= 0.05f; break;
        case 'L': g_fLightRadius += 0.05f; break;
        case 'J': g_fLightRadius -= 0.05f; break;

        case 'o': g_fGaussianRoughness += 0.1f; break;
        case 'u': g_fGaussianRoughness -= 0.1f; break;
        case 'O': g_fGaussianRoughness += 0.01f; break;
        case 'U': g_fGaussianRoughness -= 0.01f; break;

        case 'y': g_bDrawLightSource = !g_bDrawLightSource; break;
        case 't': g_bScaleCyl = !g_bScaleCyl; break;
        case 'b': g_LightTimer.TogglePause(); break;
        case 'g': g_bDrawDark = !g_bDrawDark; break;

    }
    if(g_fLightRadius < 0.2f) {
        g_fLightRadius = 0.2f;
    }
    g_fGaussianRoughness = std::max(0.00001f, g_fGaussianRoughness);
    g_fGaussianRoughness = std::min(1.0f, g_fGaussianRoughness);
}

GLUSvoid terminate(GLUSvoid)
{
    delete g_pPlaneMesh;
    delete g_pCylinderMesh;
    delete g_pCubeMesh;
}

int main(int argc, char* argv[])
{
    EGLint eglConfigAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint eglContextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    glusWindowSetInitFunc(init);
    glusWindowSetReshapeFunc(reshape);
    glusWindowSetUpdateFunc(display);
    glusWindowSetTerminateFunc(terminate);
    glusWindowSetKeyFunc(keyboard);

    if (!glusWindowCreate("Main Window", 480, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes)) {
        return -1;
    }

    glusWindowRun();

    return 0;
}

