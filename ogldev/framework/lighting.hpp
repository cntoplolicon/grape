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

    void setBaseLight(const BaseLight &light) const
    {
        glUniform3fv(color, 1, light.color.const_value_ptr());
        glUniform1f(ambientIntensity, light.ambientIntensity);
        glUniform1f(diffuseIntensity, light.diffuseIntensity);
    }
};

struct DirectionalLightUnform : public BaseLineUniform
{
    GLuint direction;
    void setDirectionalLight(const DirectionalLight &light) const
    {
        BaseLineUniform::setBaseLight(light);
        glUniform3fv(direction, 1, light.direction.const_value_ptr());
    }
};

struct AttenuationUniform
{
    GLuint constant;
    GLuint linear;
    GLuint quadratic;

    void setAttenuation(Attenuation attenuation) const
    {
        glUniform1f(constant, attenuation.constant);
        glUniform1f(linear, attenuation.linear);
        glUniform1f(quadratic, attenuation.quadratic);
    }
};

struct PointLightUniform : public BaseLineUniform
{
    GLuint position;
    AttenuationUniform attenuation;

    void setPointLight(const PointLight &light) const
    {
        BaseLineUniform::setBaseLight(light);
        attenuation.setAttenuation(light.attenuation);
        glUniform3fv(position, 1, light.position.const_value_ptr());
    }
};

struct SpotLightUniform : public PointLightUniform
{
    GLuint direction;
    GLuint cutoff;

    void setSpotLight(const SpotLight &light) const
    {
        PointLightUniform::setPointLight(light);
        glUniform3fv(direction, 1, light.direction.const_value_ptr());
        glUniform1f(cutoff, light.cutoff);
    }
};

struct SpecularUniform
{
    GLuint specularIntensity;
    GLuint shiness;
};

#endif
