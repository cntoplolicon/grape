#include "GL/glus.h"
#include "glutil/glutil.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glimg/glimg.h"
#include "mesh.hpp"
#include "timer.hpp"
#include "mouse_pole.hpp"
#include "uniform_block_array.hpp"

struct ProgramData
{
    GLuint program;
    GLuint modelViewUnif;
};

float g_fzNear = 1.0f;
float g_fzFar = 1000.0f;

ProgramData g_program;

const int g_projectionBlockIndex = 0;
const int g_colorTexUnit = 0;

ProgramData loadProgram(const std::string &vertShaderFile, const std::string &fragShaderFile)
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUSprogram program;

    glusFileLoadText(vertShaderFile.c_str(), &vertexSource);
    glusFileLoadText(fragShaderFile.c_str(), &fragmentSource);
    glusProgramBuildFromSource(&program, const_cast<const GLUSchar**>(&vertexSource.text), 0, 0, 0, const_cast<const GLUSchar**>(&fragmentSource.text));

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    ProgramData programData;
    programData.program = program.program;
    programData.modelViewUnif = glGetUniformLocation(program.program, "modelViewMatrix");
    GLuint projectionUnif = glGetUniformBlockIndex(program.program, "Projection");
    glUniformBlockBinding(program.program, projectionUnif, g_projectionBlockIndex);

    return programData;
}

GLuint g_projectionUniformBuffer = 0;
GLuint g_checkerTexture = 0;
GLuint g_mipmapTestTexture = 0;

const int NUM_SAMPLERS = 6;
GLuint g_samplers[NUM_SAMPLERS];

void initSamplers()
{
    glGenSamplers(NUM_SAMPLERS, g_samplers);

    for (int i = 0; i < NUM_SAMPLERS; i++) {
        glSamplerParameteri(g_samplers[i], GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(g_samplers[i], GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    //Nearest
    glSamplerParameteri(g_samplers[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(g_samplers[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //Linear
    glSamplerParameteri(g_samplers[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g_samplers[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //Linear mipmap Nearest
    glSamplerParameteri(g_samplers[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g_samplers[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    //Linear mipmap linear
    glSamplerParameteri(g_samplers[3], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g_samplers[3], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //Low anisotropic
    glSamplerParameteri(g_samplers[4], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g_samplers[4], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameterf(g_samplers[4], GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);

    //Max anisotropic
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    glSamplerParameteri(g_samplers[5], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g_samplers[5], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameterf(g_samplers[5], GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
}

void fillWithColor(std::vector<GLubyte> &buffer, GLubyte red, GLubyte green, GLubyte blue, int width, int height)
{
    int numTexels = width * height;
    buffer.resize(numTexels * 3);

    for (std::vector<GLubyte>::iterator it = buffer.begin(); it != buffer.end();) {
        *it++ = red;
        *it++ = green;
        *it++ = blue;
    }
}

const GLubyte mipmapColors[] =
{
    0xFF, 0xFF, 0x00,
    0xFF, 0x00, 0xFF,
    0x00, 0xFF, 0xFF,
    0xFF, 0x00, 0x00,
    0x00, 0xFF, 0x00,
    0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
};

void initdMipmapTexture()
{
    glGenTextures(1, &g_mipmapTestTexture);
    glBindTexture(GL_TEXTURE_2D, g_mipmapTestTexture);

    GLint oldAlign = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldAlign);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (int mipmapLevel = 0; mipmapLevel < 8; mipmapLevel++) {
        int width = 128 >> mipmapLevel;
        int height = 128 >> mipmapLevel;
        std::vector<GLubyte> buffer;

        const GLubyte *pCurrColor = &mipmapColors[mipmapLevel * 3];
        fillWithColor(buffer, pCurrColor[0], pCurrColor[1], pCurrColor[2], width, height);

        glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGB8, width, height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, &buffer[0]);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, oldAlign);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 7);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initCheckerTexture()
{
    try {
        std::auto_ptr<glimg::ImageSet> pImageSet(glimg::loaders::dds::LoadFromFile("./checker.dds"));

        glGenTextures(1, &g_checkerTexture);
        glBindTexture(GL_TEXTURE_2D, g_checkerTexture);

        for (int mipmapLevel = 0; mipmapLevel < pImageSet->GetMipmapCount(); mipmapLevel++) {
            glimg::SingleImage image = pImageSet->GetImage(mipmapLevel, 0, 0);
            glimg::Dimensions dims = image.GetDimensions();
            glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGB8, dims.width, dims.height, 0,
                    GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image.GetImageData());
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pImageSet->GetMipmapCount() - 1);
        glBindTexture(GL_TEXTURE_2D, 0);
    } catch (std::exception &e) {
        printf("%s\n", e.what());
        throw;
    }
}

Mesh *g_pPlane = NULL;
Mesh *g_pCorridor = NULL;

struct ProjectionBlock
{
    GLfloat projection[16];
};

GLUSboolean init()
{
    g_program = loadProgram("./position_texture.vert", "./texture_color.frag");

    g_pCorridor = new Mesh("./model/Corridor.xml");
    g_pPlane = new Mesh("./model/BigPlane.xml");

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

    initCheckerTexture();
    initdMipmapTexture();
    initSamplers();

    return GLUS_TRUE;
}

void reshape(int width, int height)
{
    ProjectionBlock projectionBlock;
    glusMatrix4x4Perspectivef(projectionBlock.projection, 90.0f, (float)width / (float)height, g_fzNear, g_fzFar);

    glBindBuffer(GL_UNIFORM_BUFFER, g_projectionUniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ProjectionBlock), &projectionBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glViewport(0, 0, width, height);
}

Timer g_camTimer = Timer(Timer::TT_LOOP, 5.0f);
int g_currSampler = 0;

bool g_useMipmapTexture = false;
bool g_drawCorridor = false;

GLUSboolean display(GLUSfloat time)
{
    glClearColor(0.75f, 0.75f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_camTimer.Update(time);

    float cyclicAngle = g_camTimer.GetAlpha() * 6.28f;
    float hOffset = cosf(cyclicAngle) * 0.25f;
    float vOffset = sinf(cyclicAngle) * 0.25f;

    GLfloat modelViewMatrix[16];
    glusMatrix4x4LookAtf(modelViewMatrix ,
            hOffset, 1.0f, -64.0f,
            hOffset, -5.0f + vOffset, -44.0f,
            0.0f, 1.0f, 0.0f);

    glUseProgram(g_program.program);
    glUniformMatrix4fv(g_program.modelViewUnif, 1, GL_FALSE, modelViewMatrix);

    glActiveTexture(GL_TEXTURE0 + g_colorTexUnit);
    glBindTexture(GL_TEXTURE_2D, g_useMipmapTexture ? g_mipmapTestTexture : g_checkerTexture);
    glBindSampler(g_colorTexUnit, g_samplers[g_currSampler]);

    Mesh *mesh = g_drawCorridor ? g_pCorridor : g_pPlane;
    mesh->render("tex");

    glBindSampler(g_colorTexUnit, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);

    return GLUS_TRUE;
}

const char *g_samplerNames[NUM_SAMPLERS] =
{
	"Nearest",
	"Linear",
	"Linear with nearest mipmaps",
	"Linear with linear mipmaps",
	"Low anisotropic",
	"Max anisotropic",
};

void keyboard(const GLUSboolean pressed, const GLUSint key)
{
    if (!pressed) {
        return;
    }

	switch (key)
	{
	case 32:
		g_useMipmapTexture = !g_useMipmapTexture;
		break;
	case 'y':
		g_drawCorridor = !g_drawCorridor;
		break;
	case 'p':
		g_camTimer.TogglePause();
		break;
	}

	if(('1' <= key) && (key <= '9')) {
		int number = key - '1';
		if(number < NUM_SAMPLERS) {
			printf("Sampler: %s\n", g_samplerNames[number]);
			g_currSampler = number;
		}
	}
}

GLUSvoid terminate()
{
    delete g_pPlane;
    delete g_pCorridor;
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

