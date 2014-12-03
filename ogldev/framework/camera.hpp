#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <algorithm>
#include "GL/glus.h"
#include "math.hpp"

class Camera
{
    const float STEP = 0.1f;

    GLUSfloat position[3];
    GLUSfloat direction[3];
    GLUSfloat up[3];

    GLUSfloat theta = 0.0f;
    GLUSfloat phi = 0.0f;

public:
    Camera() 
    {
        SetPosition(0.0f, 0.0f, 0.0f);
        SetDirection(0.0f, 0.0f, 1.0f);
        SetUpDireciton(0.0f, 1.0f, 0.0f);
    }

    void SetPosition(float x, float y, float z) 
    {
        GLfloat values[] = {x, y, z};
        std::copy(values, values + 3, position);
    }

    void SetDirection(float x, float y, float z) 
    {
        GLfloat values[] = {x, y, z};
        std::copy(values, values + 3, direction);
        glusVector3Normalizef(direction);
    }

    void SetUpDireciton(float x, float y, float z) 
    {
        GLfloat values[] = {x, y, z};
        std::copy(values, values + 3, up);
        glusVector3Normalizef(up);
    }

    void GetMatrix(GLUSfloat *result)
    {
        GLfloat target[3];
        glusVector3AddVector3f(target, position, direction);
        glusMatrix4x4LookAtf(result, position[0], position[1], position[2], 
                target[0], target[1], target[2], up[0], up[1], up[2]);
    }

    Vector3f GetPosition()
    {
        return {position[0], position[1], position[2]};
    }

    Vector3f GetDirection()
    {
        return {direction[0], direction[1], direction[2]};
    }

    Vector3f GetUpDirection()
    {
        return {up[0], up[1], up[2]};
    }

    void OnKey(GLUSboolean pressed, GLUSint key) 
    {
        if (!pressed) {
            return;
        }
        GLUSfloat step[3] = {0.0f, 0.0f, 0.0f};
        switch (key) {
            case 'w':
                glusVector3MultiplyScalarf(step, direction, STEP);
                break;
            case 's':
                glusVector3MultiplyScalarf(step, direction, -STEP);
                break;
            case 'a':
                glusVector3Crossf(step, direction, up);
                glusVector3MultiplyScalarf(step, step, -STEP);
                break;
            case 'd':
                glusVector3Crossf(step, direction, up);
                glusVector3MultiplyScalarf(step, step, STEP);
                break;
            default:
                break;
        }
        glusVector3AddVector3f(position, position, step);
    }

    void UpdateDirection(GLUSint x, GLUSint y)
    {
        GLUSfloat forward[3] = {0.0f, 0.0f, 1.0f};
        theta += -x / 20.0f;
        phi += y / 20.0f;
        phi = glusMathClampf(phi, -90.0f, 90.0f);
        GLUSfloat rotation[16];
        glusMatrix4x4Identityf(rotation);
        glusMatrix4x4RotateRyf(rotation, theta);
        glusMatrix4x4RotateRxf(rotation, phi);
        glusMatrix4x4MultiplyVector3f(direction, rotation, forward);
    }

    void OnMouse(GLUSint buttons, GLUSint x, GLUSint y)
    {
        static bool initialized = false;
        static GLUSint mouseX, mouseY;
        if (initialized) {
            UpdateDirection(x - mouseX, y - mouseY);
        }
        initialized = true;
        mouseX = x;
        mouseY = y;
    }
};

#endif
