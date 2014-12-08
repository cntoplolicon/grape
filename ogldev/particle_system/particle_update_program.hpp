#ifndef _PARTICLE_UPDATE_PROGRAM_H_
#define _PARTICLE_UPDATE_PROGRAM_H_

#include "GL/glus.h"

struct ParticleUpdateProgram
{
    GLuint program;
    GLuint deltaTime;
    GLuint time;
    GLuint randomTextureSampler;
    GLuint launcherLifetime;
    GLuint shellLifetime;
    GLuint secondShellLifetime;

    void loadUniforms(GLuint program)
    {
        this->program = program;
        deltaTime = glGetUniformLocation(program, "deltaTime");
        time = glGetUniformLocation(program, "time");
        randomTextureSampler = glGetUniformLocation(program, "randomTextureSampler");
        launcherLifetime = glGetUniformLocation(program, "launcherLifetime");
        shellLifetime = glGetUniformLocation(program, "shellLifetime");
        secondShellLifetime = glGetUniformLocation(program, "secondShellLifetime");
    }

    void setLifetime(float launcherLifetime, float shellLifetime, float secondShellLifetime)
    {
        glUniform1f(this->launcherLifetime, launcherLifetime);
        glUniform1f(this->shellLifetime, shellLifetime);
        glUniform1f(this->secondShellLifetime, secondShellLifetime);
    }

    void setTime(float deltaTime, float time)
    {
        glUniform1f(this->deltaTime, deltaTime);
        glUniform1f(this->time, time);
    }
};

#endif
