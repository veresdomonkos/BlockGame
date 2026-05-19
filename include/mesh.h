#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>

typedef struct Mesh
{
    GLuint vao;
    GLuint vbo;
    int vertexCount;
}
Mesh;

typedef enum Face
{
    FACE_FRONT, FACE_BACK, FACE_RIGHT, FACE_LEFT, FACE_UP, FACE_DOWM
}
Face;

void vertexData_append_block_face(GLfloat *vertexData, int *vertexIndex, int x, int y, int z, Face face, int uvOffsetX, int uvOffsetY);
void mesh_delete(Mesh *mesh);

#endif