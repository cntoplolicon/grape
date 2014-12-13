#ifndef __LIGHTING_PROGRAM__
#define __LIGHTING_PROGRAM__

#include <cassert>
#include "lighting.hpp"
#include "GL/glus.h"

template<int MAX_POINT_LIGHTS, int MAX_SPOT_LIGHTS>
class LightingProgram
{
public:
    GLuint program;
    GLuint viewMatrix;
    GLuint modelViewMatrix;
    GLuint modelViewMatrixForNormal;
    GLuint projectionMatrix;
    GLuint textureSampler;
    GLuint normalSampler;

    DirectionalLightUnform directionalLight;
    GLuint numPointLights;
    PointLightUniform pointLights[MAX_POINT_LIGHTS];
    GLuint numSpotLights;
    SpotLightUniform spotLights[MAX_SPOT_LIGHTS];
    SpecularUniform specular;

    virtual void loadUniforms(GLuint program)
    {
        this->program = program;

        modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
        viewMatrix = glGetUniformLocation(program, "viewMatrix");
        modelViewMatrixForNormal = glGetUniformLocation(program, "modelViewMatrixForNormal");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");

        textureSampler = glGetUniformLocation(program, "textureSampler");
        normalSampler = glGetUniformLocation(program, "normalSampler");

        specular.specularIntensity = glGetUniformLocation(program, "specularIntensity");
        specular.shiness = glGetUniformLocation(program, "shiness");

        directionalLight.color = glGetUniformLocation(program, "directionalLight.base.color");
        directionalLight.ambientIntensity = glGetUniformLocation(program, "directionalLight.base.ambientIntensity");
        directionalLight.diffuseIntensity = glGetUniformLocation(program, "directionalLight.base.diffuseIntensity");
        directionalLight.direction = glGetUniformLocation(program, "directionalLight.direction");

        numPointLights = glGetUniformLocation(program, "numPointLights");
        for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
            char buffer[256];
            sprintf(buffer, "pointLights[%d].base.color", i);
            pointLights[i].color = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].base.ambientIntensity", i);
            pointLights[i].ambientIntensity = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].base.diffuseIntensity", i);
            pointLights[i].diffuseIntensity = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].position", i);
            pointLights[i].position = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].attenuation.constant", i);
            pointLights[i].attenuation.constant = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].attenuation.linear", i);
            pointLights[i].attenuation.linear = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].attenuation.quadratic", i);
            pointLights[i].attenuation.quadratic= glGetUniformLocation(program, buffer);
        }
        numSpotLights = glGetUniformLocation(program, "numSpotLights");
        for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
            char buffer[256];
            sprintf(buffer, "spotLights[%d].base.base.color", i);
            spotLights[i].color = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.base.ambientIntensity", i);
            spotLights[i].ambientIntensity = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.base.diffuseIntensity", i);
            spotLights[i].diffuseIntensity = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.position", i);
            spotLights[i].position = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.attenuation.constant", i);
            spotLights[i].attenuation.constant = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.attenuation.linear", i);
            spotLights[i].attenuation.linear = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.attenuation.quadratic", i);
            spotLights[i].attenuation.quadratic = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].direction", i);
            spotLights[i].direction = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].cutoff", i);
            spotLights[i].cutoff = glGetUniformLocation(program, buffer);
        }
    }

    void setDirectionalLight(const DirectionalLight &light) const
    {
        this->directionalLight.setDirectionalLight(light);
    }

    void setPointLights(const PointLight *lights, int n) const
    {
        glUniform1i(this->numPointLights, n);
        for (int i = 0; i < n; i++) {
            pointLights[i].setPointLight(lights[i]);
        }
    }

    void setSpotLights(const SpotLight *lights, int n) const
    {
        glUniform1i(this->numSpotLights, n);
        for (int i = 0; i < n; i++) {
            spotLights[i].setSpotLight(lights[i]);
        }
    }
};


#endif
