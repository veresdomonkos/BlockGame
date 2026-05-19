#include "input.h"
#include <stdio.h>

vec2 mouseOffset;
vec3 directions;
int mouseButtons[2];

void input_update_mouse_offset(GLFWwindow *window, int width, int height)
{
    double xPos;
    double yPos;

    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, width / 2, height / 2);


    //printf("%f %f",xPos, yPos);
    mouseOffset[0] = xPos - (double)width / 2;
    mouseOffset[1] = (double)height / 2 - yPos;
}

void input_update_directions(GLFWwindow *window)
{
    directions[0] = 0;
    directions[1] = 0;
    directions[2] = 0;

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        directions[0] += 1;
    }

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        directions[0] -= 1;
    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        directions[2] -= 1;
    }

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        directions[2] += 1;
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        directions[1] += 1;
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        directions[1] -= 1;
    }

}

void input_update_mouse_buttons(GLFWwindow *window)
{
    static bool canRight;
    static bool canLeft;

    int currentRightPress = glfwGetMouseButton(window, 1);
    int currentLeftPress = glfwGetMouseButton(window, 0);

    mouseButtons[0] = 0;
    mouseButtons[1] = 0;

    if(!canRight && currentRightPress == GLFW_RELEASE)
    {
        canRight = true;
    }

    if(!canLeft && currentLeftPress == GLFW_RELEASE)
    {
        canLeft = true;
    }

    if(canRight && currentRightPress == GLFW_PRESS)
    {
        canRight = false;
        mouseButtons[1] = 1;
    }

    if(canLeft && currentLeftPress == GLFW_PRESS)
    {
        canLeft = false;
        mouseButtons[0] = 1;
    }
}