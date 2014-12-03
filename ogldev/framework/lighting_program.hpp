#ifndef __LIGHTING_PROGRAM__
#define __LIGHTING_PROGRAM__

#include "lighting.hpp"
#include "GL/glus.h"

template<int MAX_POINT_LIGHTS, int MAX_SPOT_LIGHTS>
class LightingProgram
{
public:
    GLuint program;
    GLuint position;
    GLuint texCoord;
    GLuint normal;
    GLuint modelViewMatrix;
    GLuint modelViewMatrixForNormal;
    GLuint projectionMatrix;
    GLuint textureSampler;

    DirectionalLightUnform directionalLight;
    GLuint numPointLights;
    PointLightUniform pointLights[MAX_POINT_LIGHTS];
    GLuint numSpotLights;
    SpotLightUniform spotLights[MAX_SPOT_LIGHTS];
    SpecularUniform specular;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        position = glGetAttribLocation(program, "position");
        texCoord = glGetAttribLocation(program, "texCoord");
        normal = glGetAttribLocation(program, "normal");

        modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
        modelViewMatrixForNormal = glGetUniformLocation(program, "modelViewMatrixForNormal");
        projectionMatrix = glGetUniformLocation(program, "projectionMatrix");

        textureSampler = glGetUniformLocation(program, "textureSampler");

        specular.specularIntensity = glGetUniformLocation(program, "specularIntensity");
        specular.shiness = glGetUniformLocation(program, "shiness");

        directionalLight.base.color = glGetUniformLocation(program, "directionalLight.base.color");
        directionalLight.base.ambientIntensity = glGetUniformLocation(program, "directionalLight.base.ambientIntensity");
        directionalLight.base.diffuseIntensity = glGetUniformLocation(program, "directionalLight.base.diffuseIntensity");
        directionalLight.direction = glGetUniformLocation(program, "directionalLight.direction");

        numPointLights = glGetUniformLocation(program, "numPointLights");
        for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
            char buffer[256];
            sprintf(buffer, "pointLights[%d].base.color", i);
            pointLights[i].base.color = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].base.ambientIntensity", i);
            pointLights[i].base.ambientIntensity = glGetUniformLocation(program, buffer);
            sprintf(buffer, "pointLights[%d].base.diffuseIntensity", i);
            pointLights[i].base.diffuseIntensity = glGetUniformLocation(program, buffer);
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
            spotLights[i].base.base.color = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.base.ambientIntensity", i);
            spotLights[i].base.base.ambientIntensity = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.base.diffuseIntensity", i);
            spotLights[i].base.base.diffuseIntensity = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.position", i);
            spotLights[i].base.position = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.attenuation.constant", i);
            spotLights[i].base.attenuation.constant = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.attenuation.linear", i);
            spotLights[i].base.attenuation.linear = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].base.attenuation.quadratic", i);
            spotLights[i].base.attenuation.quadratic = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].direction", i);
            spotLights[i].direction = glGetUniformLocation(program, buffer);
            sprintf(buffer, "spotLights[%d].cutoff", i);
            spotLights[i].cutoff = glGetUniformLocation(program, buffer);
        }
    }

    void setDirectionalLight(const DirectionalLight &directionalLight) const
    {
        glUniform3fv(this->directionalLight.base.color, 1, directionalLight.base.color.const_value_ptr());
        glUniform1f(this->directionalLight.base.ambientIntensity, directionalLight.base.ambientIntensity);
        glUniform1f(this->directionalLight.base.diffuseIntensity, directionalLight.base.diffuseIntensity);
        glUniform3fv(this->directionalLight.direction, 1, directionalLight.direction.const_value_ptr());
    }

    void setPointLights(const PointLight *pointLights, int n) const
    {
        glUniform1i(this->numPointLights, n);
        for (int i = 0; i < n; i++) {
            glUniform3fv(this->pointLights[i].base.color, 1, pointLights[i].base.color.const_value_ptr());
            glUniform1f(this->pointLights[i].base.ambientIntensity, pointLights[i].base.ambientIntensity);
            glUniform1f(this->pointLights[i].base.diffuseIntensity, pointLights[i].base.diffuseIntensity);
            glUniform3fv(this->pointLights[i].position, 1, pointLights[i].position.const_value_ptr());
            glUniform1f(this->pointLights[i].attenuation.constant, pointLights[i].attenuation.constant);
            glUniform1f(this->pointLights[i].attenuation.linear, pointLights[i].attenuation.linear);
            glUniform1f(this->pointLights[i].attenuation.quadratic, pointLights[i].attenuation.quadratic);
        }
    }

    void setSpotLights(const SpotLight *spotLights, int n) const
    {
        glUniform1i(this->numSpotLights, n);
        for (int i = 0; i < n; i++) {
            glUniform3fv(this->spotLights[i].base.base.color, 1, spotLights[i].base.base.color.const_value_ptr());
            glUniform1f(this->spotLights[i].base.base.ambientIntensity, spotLights[i].base.base.ambientIntensity);
            glUniform1f(this->spotLights[i].base.base.diffuseIntensity, spotLights[i].base.base.diffuseIntensity);
            glUniform3fv(this->spotLights[i].base.position, 1, spotLights[i].base.position.const_value_ptr());
            glUniform1f(this->spotLights[i].base.attenuation.constant, spotLights[i].base.attenuation.constant);
            glUniform1f(this->spotLights[i].base.attenuation.linear, spotLights[i].base.attenuation.linear);
            glUniform1f(this->spotLights[i].base.attenuation.quadratic, spotLights[i].base.attenuation.quadratic);
            glUniform3fv(this->spotLights[i].direction, 1, spotLights[i].direction.const_value_ptr());
            glUniform1f(this->spotLights[i].cutoff, spotLights[i].cutoff);
        }
    }
};


#endif
