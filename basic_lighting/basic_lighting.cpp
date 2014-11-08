#include "GL/glus.h"
#include "glutil/glutil.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "mesh.hpp"
#include "mouse_pole.hpp"

#define ARRAY_COUNT( array ) (sizeof( array ) / (sizeof( array[0] ) * (sizeof( array ) != sizeof(void*) || sizeof( array[0] ) <= sizeof(void*))))

struct ProgramData
{
	GLuint theProgram;

	GLuint dirToLightUnif;
	GLuint lightIntensityUnif;
    GLuint ambientIntensityUnif;

	GLuint modelViewUnif;
	GLuint modelViewForNormalUnif;
};

float g_fzNear = 1.0f;
float g_fzFar = 1000.0f;


ProgramData g_WhiteDiffuseColor;
ProgramData g_VertexDiffuseColor;

const int g_projectionBlockIndex = 2;

ProgramData LoadProgram(const std::string &strVertexShader, const std::string &strFragmentShader)
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram program;

    glusFileLoadText(strVertexShader.c_str(), &vertexSource);
    glusFileLoadText(strFragmentShader.c_str(), &fragmentSource);
    glusProgramBuildFromSource(&program, const_cast<const GLUSchar**>(&vertexSource.text), 0, 0, 0, const_cast<const GLUSchar**>(&fragmentSource.text));

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

	ProgramData data;
	data.theProgram = program.program;
	data.modelViewUnif = glGetUniformLocation(data.theProgram, "modelViewMatrix");
	data.modelViewForNormalUnif = glGetUniformLocation(data.theProgram, "modelViewMatrixForNormal");
	data.dirToLightUnif = glGetUniformLocation(data.theProgram, "dirToLight");
	data.lightIntensityUnif = glGetUniformLocation(data.theProgram, "lightIntensity");
    data.ambientIntensityUnif = glGetUniformLocation(data.theProgram, "ambientIntensity");

	GLuint projectionBlock = glGetUniformBlockIndex(data.theProgram, "Projection");
	glUniformBlockBinding(data.theProgram, projectionBlock, g_projectionBlockIndex);

	return data;
}

void InitializeProgram()
{
	g_WhiteDiffuseColor = LoadProgram("./diffuse_white.glsl", "./fragment.glsl");
	g_VertexDiffuseColor = LoadProgram("./diffuse_color.glsl", "./fragment.glsl");
}

Mesh *g_pCylinderMesh = NULL;
Mesh *g_pPlaneMesh = NULL;

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

namespace
{
	void MouseButton(GLUSboolean pressed, GLUSint button, GLUSint x, GLUSint y)
	{
		forwardMouseClickToMousePole(g_viewPole, pressed, button, x, y);
		forwardMouseClickToMousePole(g_objtPole, pressed, button, x, y);
	}

	void MouseMotion(GLUSint buttons, GLUSint x, GLUSint y)
	{
		forwardMouseMotionToMousePole(g_viewPole, x, y);
		forwardMouseMotionToMousePole(g_objtPole, x, y);
    }

	void MouseWheel(GLUSint wheel, GLUSint ticks, GLUSint x, GLUSint y)
	{
		forwardMouseWheelToMousePole(g_viewPole, wheel, ticks, x, y);
		forwardMouseWheelToMousePole(g_objtPole, wheel, ticks, x, y);
	}
}

GLuint g_projectionUniformBuffer = 0;

struct ProjectionBlock
{
	glm::mat4 cameraToClipMatrix;
};

GLUSboolean init()
{
	InitializeProgram();

    g_pCylinderMesh = new Mesh("./model/UnitCylinder.xml");
    g_pPlaneMesh = new Mesh("./model/LargePlane.xml");

    glusWindowSetMouseFunc(MouseButton);
    glusWindowSetMouseMoveFunc(MouseMotion);
    glusWindowSetMouseWheelFunc(MouseWheel);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);

	glGenBuffers(1, &g_projectionUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionBlock), NULL, GL_DYNAMIC_DRAW);

	//Bind the static buffers.
	glBindBufferRange(GL_UNIFORM_BUFFER, g_projectionBlockIndex, g_projectionUniformBuffer,
		0, sizeof(ProjectionBlock));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return GLUS_TRUE;
}

glm::vec4 g_lightDirection(0.866f, 0.5f, 0.0f, 0.0f);

GLUSboolean display(GLUSfloat time)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glutil::MatrixStack modelMatrix;
		modelMatrix.SetMatrix(g_viewPole.CalcMatrix());

		glm::vec4 lightDirCameraSpace = modelMatrix.Top() * g_lightDirection;

		glUseProgram(g_WhiteDiffuseColor.theProgram);
		glUniform3fv(g_WhiteDiffuseColor.dirToLightUnif, 1, glm::value_ptr(lightDirCameraSpace));
		glUseProgram(g_VertexDiffuseColor.theProgram);
		glUniform3fv(g_VertexDiffuseColor.dirToLightUnif, 1, glm::value_ptr(lightDirCameraSpace));
		glUseProgram(0);

        //normMatrix = glm::transpose(glm::inverse(normMatrix));

		{
			glutil::PushStack push(modelMatrix);

			//Render the ground plane.
			{
				glutil::PushStack push(modelMatrix);

				glUseProgram(g_WhiteDiffuseColor.theProgram);
                glm::mat3 normMatrix(modelMatrix.Top());
                normMatrix = glm::transpose(glm::inverse(normMatrix));
				glUniformMatrix4fv(g_WhiteDiffuseColor.modelViewUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix.Top()));
				glUniformMatrix3fv(g_WhiteDiffuseColor.modelViewForNormalUnif, 1, GL_FALSE, glm::value_ptr(normMatrix));
				glUniform4f(g_WhiteDiffuseColor.lightIntensityUnif, 1.0f, 1.0f, 1.0f, 1.0f);
				g_pPlaneMesh->render();

				glUseProgram(0);
			}

			//Render the Cylinder
			{
				glutil::PushStack push(modelMatrix);

				modelMatrix.ApplyMatrix(g_objtPole.CalcMatrix());

				glUseProgram(g_VertexDiffuseColor.theProgram);
                glm::mat3 normMatrix(modelMatrix.Top());
                normMatrix = glm::transpose(glm::inverse(normMatrix));
                glUniformMatrix4fv(g_VertexDiffuseColor.modelViewUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix.Top()));
                glUniformMatrix3fv(g_VertexDiffuseColor.modelViewForNormalUnif, 1, GL_FALSE, glm::value_ptr(normMatrix));
                glUniform4f(g_VertexDiffuseColor.lightIntensityUnif, 0.8f, 0.8f, 0.8f, 1.0f);
                glUniform4f(g_VertexDiffuseColor.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
                g_pCylinderMesh->render();

                glUseProgram(0);
            }
		}

    return GLUS_TRUE;
}

void reshape (int w, int h)
{
	glutil::MatrixStack persMatrix;
	persMatrix.Perspective(45.0f, (w / (float)h), g_fzNear, g_fzFar);

	ProjectionBlock projData;
	projData.cameraToClipMatrix = persMatrix.Top();

	glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ProjectionBlock), &projData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void keyboard(const GLUSboolean pressed, const GLUSint key)
{
    if (!pressed) {
        return;
    }
}

GLUSvoid terminate(GLUSvoid)
{
    delete g_pPlaneMesh;
    delete g_pCylinderMesh;
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
