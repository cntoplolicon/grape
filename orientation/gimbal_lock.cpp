#include "GL/glus.h"
#include "mesh.hpp"

const float zNear = 1.0f, zFar = 600.0f, fov = 20.0f;
GLUSprogram program;

GLuint modelViewLocation;
GLuint projectionLocation;
GLuint baseColorLocation;

enum GimbalAxis
{
	GIMBAL_X_AXIS,
	GIMBAL_Y_AXIS,
	GIMBAL_Z_AXIS,
};

void initProgram()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    glusFileLoadText("./vertex.glsl", &vertexSource);
    glusFileLoadText("./fragment.glsl", &fragmentSource);
    glusProgramBuildFromSource(&program, const_cast<const GLUSchar**>(&vertexSource.text), 0, 0, 0, const_cast<const GLUSchar**>(&fragmentSource.text));

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    modelViewLocation = glGetUniformLocation(program.program, "modelViewMatrix");
    projectionLocation = glGetUniformLocation(program.program, "projectionMatrix");
    baseColorLocation = glGetUniformLocation(program.program, "baseColor");

    glUseProgram(program.program);

    GLfloat model[16];
    glusMatrix4x4Identityf(model);
    glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, model);

    GLfloat projection[16];
    glusMatrix4x4Perspectivef(projection, fov, 1.0f, zNear, zFar);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection);

    glUseProgram(0);
}

Mesh *gimbals[3] = {nullptr};
const char *g_strGimbalNames[3] = {
	"./model/LargeGimbal.xml",
	"./model/MediumGimbal.xml",
	"./model/SmallGimbal.xml",
};

bool g_bDrawGimbals = true;

void DrawGimbal(const GLfloat *currMatrix, GimbalAxis eAxis, const GLfloat *baseColor)
{
	if(!g_bDrawGimbals) {
		return;
    }

    GLfloat matrix[16];
    memcpy(matrix, currMatrix, sizeof(matrix));

	switch(eAxis) {
	case GIMBAL_X_AXIS:
		break;
	case GIMBAL_Y_AXIS:
        glusMatrix4x4RotateRzf(matrix, 90.0f);
        glusMatrix4x4RotateRxf(matrix, 90.0f);
		break;
	case GIMBAL_Z_AXIS:
        glusMatrix4x4RotateRyf(matrix, 90.0f);
        glusMatrix4x4RotateRxf(matrix, 90.0f);
		break;
	}

	glUseProgram(program.program);
	glUniform4fv(baseColorLocation, 1, baseColor);
	glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, matrix);
	gimbals[eAxis]->render();

	glUseProgram(0);
}

Mesh *ship;

GLUSboolean init()
{
    initProgram();

    for(int iLoop = 0; iLoop < 3; iLoop++) {
        gimbals[iLoop] = new Mesh(g_strGimbalNames[iLoop]);
    }
    ship = new Mesh("./model/Ship.xml");

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

    return GLUS_TRUE;
}

struct GimbalAngles
{
	GimbalAngles()
		: fAngleX(0.0f)
		, fAngleY(0.0f)
		, fAngleZ(0.0f)
	{}

	float fAngleX;
	float fAngleY;
	float fAngleZ;
};

GimbalAngles g_angles;

GLUSboolean update(GLUSfloat time)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat currMatrix[16] = {0.0f};
    glusMatrix4x4Translatef(currMatrix, 0.0f, 0.0f, -200.0f);
    glusMatrix4x4RotateRxf(currMatrix, g_angles.fAngleX);
    GLfloat c0[] = {0.4f, 0.4f, 1.0f, 1.0f};
	DrawGimbal(currMatrix, GIMBAL_X_AXIS, c0);
    glusMatrix4x4RotateRyf(currMatrix, g_angles.fAngleY);
    GLfloat c1[] = {0.0f, 1.0f, 0.0f, 1.0f};
	DrawGimbal(currMatrix, GIMBAL_Y_AXIS, c1);
    glusMatrix4x4RotateRzf(currMatrix, g_angles.fAngleZ);
    GLfloat c2[] = {1.0f, 0.3f, 0.3f, 1.0f};
	DrawGimbal(currMatrix, GIMBAL_Z_AXIS, c2);

	glUseProgram(program.program);

    glusMatrix4x4Scalef(currMatrix, 3.0f, 3.0f, 3.0f);
    glusMatrix4x4RotateRxf(currMatrix, -90.0f);
	glUniform4f(baseColorLocation, 1.0, 1.0, 1.0, 1.0);
	glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, currMatrix);

	ship->render("tint");

	glUseProgram(0);

    return GLUS_TRUE;
}

void reshape(GLUSint width, GLUSint height)
{
    GLfloat projection[16];
    glusMatrix4x4Perspectivef(projection, fov, (GLfloat)width / (GLfloat)height, zNear, zFar);

    glUseProgram(program.program);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection);
    glUseProgram(0);

    glViewport(0, 0, width, height);
}

void key(GLUSboolean pressed, GLUSint key)
{
    const float SMALL_ANGLE_INCREMENT = 9.0f;

    if (!pressed) {
        return;
    }
	switch (key) {
	case 'w': g_angles.fAngleX += SMALL_ANGLE_INCREMENT; break;
	case 's': g_angles.fAngleX -= SMALL_ANGLE_INCREMENT; break;

	case 'a': g_angles.fAngleY += SMALL_ANGLE_INCREMENT; break;
	case 'd': g_angles.fAngleY -= SMALL_ANGLE_INCREMENT; break;

	case 'q': g_angles.fAngleZ += SMALL_ANGLE_INCREMENT; break;
	case 'e': g_angles.fAngleZ -= SMALL_ANGLE_INCREMENT; break;

	case 32:
		g_bDrawGimbals = !g_bDrawGimbals;
		break;
	}
}

GLUSvoid terminate(GLUSvoid)
{
    delete ship;
    for (int i = 0; i < 3; i++) {
        delete gimbals[i];
    }
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
    glusWindowSetUpdateFunc(update);
    glusWindowSetTerminateFunc(terminate);
    glusWindowSetKeyFunc(key);

    if (!glusWindowCreate("Main Window", 480, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes)) {
        return -1;
    }

    glusWindowRun();

    return 0;
}

