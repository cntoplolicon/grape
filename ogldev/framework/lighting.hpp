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

struct DirectionalLight : public BaseLight
{
    Vector3f direction;
};

struct Attenuation
{
    GLfloat constant = 1.0f;
    GLfloat linear = 0.0f;
    GLfloat quadratic = 0.0f;
};

struct PointLight : public BaseLight
{
    Vector3f position;
    Attenuation attenuation;
};

struct SpotLight : public PointLight
{
    Vector3f direction;
    float cutoff;
};

struct BaseLineUniform
{
    GLuint color;
    GLuint ambientIntensity;
    GLuint diffuseIntensity;
};

struct DirectionalLightUnform : public BaseLineUniform
{
    GLuint direction;
};

struct AttenuationUniform
{
    GLuint constant;
    GLuint linear;
    GLuint quadratic;
};

struct PointLightUniform : public BaseLineUniform
{
    GLuint position;
    AttenuationUniform attenuation;
};

struct SpotLightUniform : public PointLightUniform
{
    GLuint direction;
    GLuint cutoff;
};

struct SpecularUniform
{
    GLuint specularIntensity;
    GLuint shiness;
};

#endif
