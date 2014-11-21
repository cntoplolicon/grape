#include "GL/glus.h"
#include "glutil/glutil.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "mesh.hpp"
#include "timer.hpp"
#include "mouse_pole.hpp"
#include "uniform_block_array.hpp"

struct ProgramData
{
    GLuint program;
    GLuint modelViewUnif;
    GLuint modelViewForNormalUnif;
    GLuint gaussianTextureUnif;
    GLuint useLookupTableUnif;
};

struct UnlitProgramData 
{
    GLuint program;
    GLuint modelViewUnif;
    GLuint objectColorUnif;
};

UnlitProgramData g_UnlitProgram;
ProgramData g_LightingProgram;

const int g_materialBlockIndex = 0;
const int g_lightingBlockIndex = 1;
const int g_projectionBlockIndex = 2;

GLuint g_materialUniformBuffer;
int g_materialOffset = 0;
int g_currMaterial = 0;
GLuint g_lightingUniformBuffer;
GLuint g_projectionUniformBuffer;

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

const GLuint g_TextureUnit = 1;
ProgramData loadLitProgram(const std::string &vertShaderFile, const std::string &fragShaderFile)
{
    ProgramData data;
    data.program = loadProgram(vertShaderFile, fragShaderFile);
    data.modelViewUnif = glGetUniformLocation(data.program, "modelViewMatrix");
    data.modelViewForNormalUnif = glGetUniformLocation(data.program, "modelViewMatrixForNormal");
    data.gaussianTextureUnif = glGetUniformLocation(data.program, "gaussianTexture");
    data.useLookupTableUnif = glGetUniformLocation(data.program, "useLookupTable");

    GLuint materialBlock = glGetUniformBlockIndex(data.program, "MaterialBlock");
    glUniformBlockBinding(data.program, materialBlock, g_materialBlockIndex);
    GLuint lightingBlock = glGetUniformBlockIndex(data.program, "LightingBlock");
    glUniformBlockBinding(data.program, lightingBlock, g_lightingBlockIndex);
    GLuint projectionBlock = glGetUniformBlockIndex(data.program, "Projection");
    glUniformBlockBinding(data.program, projectionBlock, g_projectionBlockIndex);

    glUseProgram(data.program);
    glUniform1i(data.gaussianTextureUnif, g_TextureUnit);
    glUseProgram(0);


    return data;
}

struct ProjectionBlock
{
    GLfloat projection[16];
};
struct MaterialBlock
{
    GLfloat diffuseColor[4];
    GLfloat specularColo[4];
    GLfloat specularShininess;
};
struct Light
{
    GLfloat cameraSpaceLightPos[4];
    GLfloat lightIntensity[4];
};

const int NUMBER_OF_LIGHTS = 2;

struct LightingBlock
{
	GLfloat ambientIntensity[4];
	GLfloat lightAttenuation;
	GLfloat padding[3];
	Light lights[NUMBER_OF_LIGHTS];
};

Mesh *g_objectMesh;
Mesh *g_cubeMesh;

bool g_useLookupTable = false;

const int NUM_MATERIALS = 3;

const float g_fHalfLightDistance = 25.0f;
const float g_fLightAttenuation = 1.0f / (g_fHalfLightDistance * g_fHalfLightDistance);
Timer g_lightTimer = Timer(Timer::TT_LOOP, 6.0f);
float g_lightHeight = 1.0f;
float g_lightRadius = 3.0f;

void calcLightPosition(GLfloat ret[])
{
	const float fScale = 3.14159f * 2.0f;

	float timeThroughLoop = g_lightTimer.GetAlpha();
	ret[0] = cosf(timeThroughLoop * fScale) * g_lightRadius;
    ret[1] = g_lightHeight;
	ret[2] = sinf(timeThroughLoop * fScale) * g_lightRadius;
    ret[3] = 1.0f;
}

void initMaterials()
{
	UniformBlockArray<MaterialBlock, NUM_MATERIALS> materials;

    materials[0] = {
        {1.0f, 0.673f, 0.043f, 1.0f},
        {1.0f * 0.4f, 0.673f * 0.4f, 0.043f * 0.4f, 1.0f * 0.4f},
        0.125f
    };
    materials[1] = {
        {0.01f, 0.01f, 0.01f, 1.0f},
        {0.99f, 0.99f, 0.99f, 1.0f},
        0.125f
    };
    materials[2] = {
        {0.01f, 0.01f, 0.01f, 1.0f},
        {0.99f, 0.99f, 0.99f, 1.0f},
        0.5f
    };

	g_materialUniformBuffer = materials.createBuffer();
	g_materialOffset = materials.offset();
}

const int cosAngleResolution = 512;
const int shininessResolution = 512;
GLuint g_Texture;
GLuint g_textureSampler;

void initTexture()
{
    GLubyte *buffer = new GLubyte[cosAngleResolution * shininessResolution];
    int k = 0;
	for(int i = 1; i <= shininessResolution; i++) {
		float shininess = i / (float)(shininessResolution);
		for(int j = 0; j < cosAngleResolution; j++) {
			float cosAng = j / (float)(cosAngleResolution - 1);
			float angle = acosf(cosAng);
			float exponent = angle / shininess;
			exponent = -(exponent * exponent);
			float gaussianTerm = exp(exponent);
            buffer[k++] = (GLubyte)(gaussianTerm * 255.0f);
		}
	}

    glGenTextures(1, &g_Texture);
    glActiveTexture(GL_TEXTURE0 + g_TextureUnit);
    glBindTexture(GL_TEXTURE_2D, g_Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, cosAngleResolution, shininessResolution, 0,
            GL_RED, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glGenSamplers(1, &g_textureSampler);
    glSamplerParameteri(g_textureSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(g_textureSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(g_textureSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(g_textureSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindSampler(g_TextureUnit, g_textureSampler);

    delete[] buffer;
}

GLUSboolean init()
{
    g_UnlitProgram = loadUnlitProgram("./position.vert", "./mono.frag");
    g_LightingProgram = loadLitProgram("./position_normal.vert", "./lighting.frag");

    g_objectMesh = new Mesh("./model/Infinity.xml");
    g_cubeMesh = new Mesh("./model/UnitCube.xml");

    initMaterials();
    initTexture();

    glGenBuffers(1, &g_projectionUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionBlock), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, g_projectionBlockIndex, g_projectionUniformBuffer, 0, sizeof(ProjectionBlock));

	glGenBuffers(1, &g_lightingUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, g_lightingUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightingBlock), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, g_lightingBlockIndex, g_lightingUniformBuffer, 0, sizeof(LightingBlock));

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

    switch (key) {
        case 'p': g_lightTimer.TogglePause(); break;
        case '-': g_lightTimer.Rewind(0.5f); break;
        case '=': g_lightTimer.Fastforward(0.5f); break;
        case 'l': g_useLookupTable = !g_useLookupTable; break;
        default: break;
    }

    if ('1' <= key && key <= '9') {
        int number = key - '1';
		if (number < NUM_MATERIALS) {
			g_currMaterial = number;
		}
	}
}

void convertMatrix4x4(GLfloat matrix[], const glm::mat4 &glmMatrix)
{
    memcpy(matrix, glm::value_ptr(glmMatrix), sizeof(GLfloat) * 16);
}

GLUSboolean display(GLUSfloat time)
{
    g_lightTimer.Update(time);

	glClearColor(0.75f, 0.75f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat modelMatrix[16];
    GLfloat viewMatrix[16];
    GLfloat modelViewMatrix[16];
    GLfloat modelViewMatrixForNormal[9];

    convertMatrix4x4(modelMatrix, g_objtPole.CalcMatrix());
    convertMatrix4x4(viewMatrix, g_viewPole.CalcMatrix());
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4Scalef(modelViewMatrix, 2.0f, 2.0f, 2.0f);
    glusMatrix4x4ExtractMatrix3x3f(modelViewMatrixForNormal, modelViewMatrix);
    glusMatrix3x3Inversef(modelViewMatrixForNormal);
    glusMatrix3x3Inversef(modelViewMatrixForNormal);

    LightingBlock lightingBlock = {
        {0.2f, 0.2f, 0.2f, 1.0f}, 
        g_fLightAttenuation,
        {0.0f, 0.0f, 0.0f},
        {
            {{0.0f, 0.0f, 0.0f, 0.0f}, {0.6f, 0.6f, 0.6f, 1.0f}},
            {{0.0f, 0.0f, 0.0f, 0.0f}, {0.4f, 0.4f, 0.4f, 1.0f}} 
        }
    };
	GLfloat globalLightDirection[] = {0.707f, 0.707f, 0.0f, 0.0f};
    glusMatrix4x4MultiplyPoint4f(lightingBlock.lights[0].cameraSpaceLightPos, viewMatrix, globalLightDirection);
    GLfloat lightPosition[4];
    calcLightPosition(lightPosition);
    glusMatrix4x4MultiplyPoint4f(lightingBlock.lights[1].cameraSpaceLightPos, viewMatrix, lightPosition);

    glBindBuffer(GL_UNIFORM_BUFFER, g_lightingUniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightingBlock), &lightingBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUseProgram(g_LightingProgram.program);
    glUniformMatrix4fv(g_LightingProgram.modelViewUnif, 1, GL_FALSE, modelViewMatrix);
    glUniformMatrix3fv(g_LightingProgram.modelViewForNormalUnif, 1, GL_FALSE, modelViewMatrixForNormal);
    glUniform1i(g_LightingProgram.useLookupTableUnif, g_useLookupTable);
    glBindBufferRange(GL_UNIFORM_BUFFER, g_materialBlockIndex, g_materialUniformBuffer, 
            g_materialOffset * g_currMaterial, sizeof(MaterialBlock));
    g_objectMesh->render();
    glBindBufferBase(GL_UNIFORM_BUFFER, g_materialBlockIndex, 0);
    glUseProgram(0);


    convertMatrix4x4(modelViewMatrix, g_viewPole.CalcMatrix());
    glusMatrix4x4Translatef(modelViewMatrix, lightPosition[0], lightPosition[1], lightPosition[2]);
    glusMatrix4x4Scalef(modelViewMatrix, 0.25f, 0.25f, 0.25f);

    glUseProgram(g_UnlitProgram.program);
    glUniformMatrix4fv(g_UnlitProgram.modelViewUnif, 1, GL_FALSE, modelViewMatrix);
    glUniform4fv(g_UnlitProgram.objectColorUnif, 1, (float []) {1.0f, 1.0f, 1.0f, 1.0f});
	g_cubeMesh->render("flat");

    convertMatrix4x4(modelViewMatrix, g_viewPole.CalcMatrix());
    glusMatrix4x4Translatef(modelViewMatrix, globalLightDirection[0] * 100.0f, globalLightDirection[1] * 100.0, globalLightDirection[2] * 100.0f);
    glusMatrix4x4Scalef(modelViewMatrix, 5.0f, 5.0f, 5.0f);
    glUniformMatrix4fv(g_UnlitProgram.modelViewUnif, 1, GL_FALSE, modelViewMatrix);

    g_cubeMesh->render("flat");
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

