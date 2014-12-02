#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <algorithm>
#include "GL/glus.h"

class Camera
{
    const float STEP = 0.1f;

    GLfloat position[3];
    GLfloat direction[3];
    GLfloat up[3];

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

    void OnKey(GLUSint key) 
    {
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
                glusVector3MultiplyScalarf(step, step, STEP);
                break;
            case 'd':
                glusVector3Crossf(step, direction, up);
                glusVector3MultiplyScalarf(step, step, -STEP);
                break;
            default:
                break;
        }
        glusVector3AddVector3f(position, position, step);
    }
};

#endif
