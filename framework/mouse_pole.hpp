#ifndef __MOUSE_POLE_H__
#define __MOUSE_POLE_H__

#include <iostream>
#include "GL/glus.h"
#include "glm/glm.hpp"
#include "glutil/MousePoles.h"

const int MOUSE_BUTTON_LEFT = 1;
const int MOUSE_BUTTON_MIDDLE = 2;
const int MOUSE_BUTTON_RIGHT = 4;

template <typename T>
inline void forwardMouseMotionToMousePole(T &pole, GLUSint x, GLUSint y)
{
    pole.MouseMove(glm::ivec2(x, y));
}

template<typename T>
inline void forwardMouseClickToMousePole(T &pole, GLUSboolean pressed, GLUSint button, GLUSint x, GLUSint y)
{
    glutil::MouseButtons eButton;
    if (button & MOUSE_BUTTON_LEFT) {
        eButton = glutil::MB_LEFT_BTN;
    }
    if (button & MOUSE_BUTTON_MIDDLE) {
        eButton = glutil::MB_MIDDLE_BTN;
    }
    if (button & MOUSE_BUTTON_RIGHT) {
        eButton = glutil::MB_RIGHT_BTN;
    }
    pole.MouseClick(eButton, pressed, 0, glm::ivec2(x, y));
}

template<typename T>
inline void forwardMouseWheelToMousePole(T &pole, GLUSint wheel, GLUSint ticks, GLUSint x, GLUSint y)
{
    static GLUSint lastTicks = 0;
    if (ticks != lastTicks) {
        pole.MouseWheel(ticks - lastTicks, 0, glm::ivec2(x, y));
    }
    lastTicks = ticks;
}

#endif

