#ifndef RENDER_H
#define RENDER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "mesh.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define RENDER_DISTANCE 32

extern GLFWwindow *window;

int initialize_window();
void configure_render();
void initialize_shaders();
void render_mesh(Mesh *mesh);
void render_update_selection(int x, int y, int z);

#endif