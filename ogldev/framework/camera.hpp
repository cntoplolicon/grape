#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <algorithm>
#include "GL/glus.h"
#include "math.hpp"

class Camera
{
    const float STEP = 0.1f;

    Vector3f position;
    Vector3f direction;
    Vector3f up;

    GLUSfloat theta = 0.0f;
    GLUSfloat phi = 0.0f;

public:
    Camera() 
    {
        setPosition(0.0f, 0.0f, 0.0f);
        setDirection(0.0f, 0.0f, 1.0f);
        setUpDireciton(0.0f, 1.0f, 0.0f);
    }

    void setPosition(float x, float y, float z) 
    {
        position = {x, y, z};
    }

    void setDirection(float x, float y, float z) 
    {
        direction = Vector3f{x, y, z}.normalized();
    }

    void setUpDireciton(float x, float y, float z) 
    {
        up = Vector3f{x, y, z}.normalized();
    }

    Matrix4x4f getMatrix()
    {
        Vector3f target = position + direction;
        return Matrix4x4f::lookAt(position, target, up);
    }

    Vector3f getPosition()
    {
        return position;
    }

    Vector3f getDirection()
    {
        return direction;
    }

    Vector3f getUpDirection()
    {
        return up;
    }

    void OnKey(GLUSboolean pressed, GLUSint key) 
    {
        if (!pressed) {
            return;
        }
        Vector3f step = {0.0f, 0.0f, 0.0f};
        switch (key) {
            case 'w':
                step = direction * STEP;
                break;
            case 's':
                step = direction * -STEP;
                break;
            case 'a':
                step = direction.cross(up);
                step = step * -STEP;
                break;
            case 'd':
                step = direction.cross(up);
                step = step * STEP;
                break;
            default:
                break;
        }
        position += step;
    }

    void UpdateDirection(GLUSint x, GLUSint y)
    {
        Vector3f forward = {0.0f, 0.0f, 1.0f};
        theta += -x / 20.0f;
        phi += y / 20.0f;
        phi = clamp(phi, -90.0f, 90.0f);
        Matrix4x4f rotation = Matrix4x4f::identity();;
        rotation = rotation.rotatey(theta);
        rotation = rotation.rotatex(phi);
        direction = rotation * forward;
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
