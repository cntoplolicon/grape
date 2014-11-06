#ifndef __MOUSE_POLE_H__
#define __MOUSE_POLE_H__

#include <iostream>
#include "GL/glus.h"
#include "glm/glm.hpp"
#include "glutil/MousePoles.h"

template <typename T>
inline void forwardMouseMotionToMousePole(T &pole, GLUSint x, GLUSint y)
{
    pole.MouseMove(glm::ivec2(x, y));
}

template<typename T>
inline void forwardMouseClickToMousePole(T &pole, GLUSboolean pressed, GLUSint button, GLUSint x, GLUSint y)
{
    glutil::MouseButtons eButton;
    switch(button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        std::cout << "left button" << std::endl;
        eButton = glutil::MB_LEFT_BTN;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        std::cout << "middle button" << std::endl;
        eButton = glutil::MB_MIDDLE_BTN;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        std::cout << "right button" << std::endl;
        eButton = glutil::MB_RIGHT_BTN;
        break;
    }
    pole.MouseClick(eButton, pressed, 0, glm::ivec2(x, y));
}

template<typename T>
inline void forwardMouseWheelToMousePole(T &pole, GLUSint wheel, GLUSint ticks, GLUSint x, GLUSint y)
{
    std::cout << "wheel: " << ticks << std::endl;
    pole.MouseWheel(ticks, 0, glm::ivec2(x, y));
}

#endif

