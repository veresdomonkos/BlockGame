#ifndef INPUT_H
#define INPUT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/vec2.h>

extern vec2 mouseOffset;
extern vec3 directions;
extern int mouseButtons[2];

void input_update_mouse_offset(GLFWwindow *window, int width, int height);
void input_update_directions(GLFWwindow *window);
void update_view(vec3 position, vec3 forward);
void input_update_mouse_buttons(GLFWwindow *window);

#endif