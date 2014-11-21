#include "GL/glus.h"
#include "glutil/glutil.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "mesh.hpp"
#include "mouse_pole.hpp"

struct ProgramData
{
    GLuint program;
    GLuint modelViewUnif;
    GLuint objectColorUnif;
};

struct UnlitProgramData 
{
    GLuint program;
    GLuint modelViewUnif;
    GLuint objectColorUnif;
};

UnlitProgramData g_UnlitProgram;
ProgramData g_LightingWhiteProgram;

GLuint g_projectionUniformBuffer;
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

UnlitProgramData loadUnlitProgram(const std::string &vertShaderFile, const std::string &fragShaderFile)
{ 
    UnlitProgramData data;
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
    data.objectColorUnif = glGetUniformLocation(data.program, "objectColor");

    GLuint projectionBlock = glGetUniformBlockIndex(data.program, "Projection");
    glUniformBlockBinding(data.program, projectionBlock, g_projectionBlockIndex);


    return data;
}

struct ProjectionBlock
{
    GLfloat projection[16];
};

Mesh *g_objectMesh;
Mesh *g_cubeMesh;

GLUSboolean init()
{
    g_UnlitProgram = loadUnlitProgram("./position.vert", "./mono.frag");
    g_LightingWhiteProgram = loadLitProgram("./position.vert", "./mono.frag");

    g_objectMesh = new Mesh("./model/Infinity.xml");
    g_cubeMesh = new Mesh("./model/UnitCube.xml");

    glGenBuffers(1, &g_projectionUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionBlock), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, g_projectionBlockIndex, g_projectionUniformBuffer, 0, sizeof(ProjectionBlock));

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    glEnable(GL_DEPTH_CLAMP);

    return GLUS_TRUE;
}

GLUSvoid terminate()
{
    delete g_objectMesh;
    delete g_cubeMesh;
}

const float fzNear = 1.0f;
const float fzFar = 1000.0f;

GLUSvoid reshape(int width, int height)
{
    ProjectionBlock projectionBlock;

    glusMatrix4x4Perspectivef(projectionBlock.projection, 45.0f, (float)width / (float)height, fzNear, fzFar);
    glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ProjectionBlock), &projectionBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glViewport(0, 0, width, height);
}

glutil::ObjectData g_initialObjectData =
{
	glm::vec3(0.0f, 0.5f, 0.0f),
	glm::fquat(1.0f, 0.0f, 0.0f, 0.0f),
};

glutil::ViewData g_initialViewData =
{
	g_initialObjectData.position,
	glm::fquat(0.92387953f, 0.3826834f, 0.0f, 0.0f),
	10.0f,
	0.0f
};

glutil::ViewScale g_viewScale =
{
	1.5f, 70.0f,
	1.5f, 0.5f,
	0.0f, 0.0f,
	90.0f / 250.0f
};

glutil::ViewPole g_viewPole = glutil::ViewPole(g_initialViewData, g_viewScale, glutil::MB_LEFT_BTN);
glutil::ObjectPole g_objtPole = glutil::ObjectPole(g_initialObjectData, 90.0f/250.0f, glutil::MB_RIGHT_BTN, &g_viewPole);

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

void keyboard(const GLUSboolean pressed, const GLUSint key)
{
    if (!pressed) {
        return;
    }
}

void convertMatrix4x4(GLfloat matrix[], const glm::mat4 &glmMatrix)
{
    memcpy(matrix, glm::value_ptr(glmMatrix), sizeof(GLfloat) * 16);
}

GLUSboolean display(GLUSfloat time)
{
	glClearColor(0.75f, 0.75f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat modelMatrix[16];
    GLfloat viewMatrix[16];
    GLfloat modelViewMatrix[16];

    convertMatrix4x4(modelMatrix, g_objtPole.CalcMatrix());
    convertMatrix4x4(viewMatrix, g_viewPole.CalcMatrix());
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);

    glUseProgram(g_LightingWhiteProgram.program);
    glUniformMatrix4fv(g_LightingWhiteProgram.modelViewUnif, 1, GL_FALSE, modelViewMatrix);
    glUniform4fv(g_LightingWhiteProgram.objectColorUnif, 1, (const GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f});
    g_objectMesh->render();
    glUseProgram(0);

    return GLUS_TRUE;
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
    glusWindowSetMouseFunc(mouseButton);
    glusWindowSetMouseMoveFunc(mouseMotion);
    glusWindowSetMouseWheelFunc(mouseWheel);

    if (!glusWindowCreate("Main Window", 480, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes)) {
        return -1;
    }

    glusWindowRun();

    return 0;
}

