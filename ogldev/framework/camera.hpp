#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <iostream>
#include "GL/glus.h"
#include "math.hpp"

class Camera
{
    const float STEP = 0.1f;

    Vector3f position;
    Vector3f direction;
    Vector3f up;

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

    void onKey(GLUSboolean pressed, GLUSint key) 
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

    void updateDirection(GLUSint x, GLUSint y)
    {
        Quaternion vert = Quaternion::rotate(-x / 20.0f, {0.0f, 1.0f, 0.0f});
        Quaternion hort = Quaternion::rotate(-y / 20.0f, direction.cross(up));
        direction = hort * vert * direction;
    }

    void onMouse(GLUSint buttons, GLUSint x, GLUSint y)
    {
        static bool initialized = false;
        static GLUSint mouseX, mouseY;
        if (initialized) {
            updateDirection(x - mouseX, y - mouseY);
        }
        initialized = true;
        mouseX = x;
        mouseY = y;
    }
};

#endif
