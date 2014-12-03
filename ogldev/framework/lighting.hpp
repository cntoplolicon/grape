#ifndef __LIGHTING_H__
#define __LIGHTING_H__

#include "GL/glus.h"
#include "math.hpp"

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

struct SpecularUniform
{
    GLuint specularIntensity;
    GLuint shiness;
};

#endif
