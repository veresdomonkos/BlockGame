#include "mesh.h"

GLfloat vertecies[] =
{
    0, 0, 1,  1, 1, 1,  0, 1, 1,  0, 0, 1,  1, 0, 1,  1, 1, 1, //front face
    1, 0, 0,  0, 1, 0,  1, 1, 0,  1, 0, 0,  0, 0, 0,  0, 1, 0, //back face
    1, 0, 1,  1, 1, 0,  1, 1, 1,  1, 0, 1,  1, 0, 0,  1, 1, 0, //right face
    0, 0, 0,  0, 1, 1,  0, 1, 0,  0, 0, 0,  0, 0, 1,  0, 1, 1, //left face
    0, 1, 1,  1, 1, 0,  0, 1, 0,  0, 1, 1,  1, 1, 1,  1, 1, 0, //up face
    1, 0, 0,  1, 0, 1,  0, 0, 1,  1, 0, 0,  0, 0, 1,  0, 0, 0  //down face
};

GLfloat faceBrightness[] =
{
    0.9f,
    0.6f,
    0.8f,
    0.7f,
    1.0f,
    0.5f
};

void vertex_get_uv(Face face, GLfloat x, GLfloat y, GLfloat z, GLfloat *uvX, GLfloat *uvY)
{
    if(face == FACE_FRONT)
    {
        *uvX = x;
        *uvY = 1 - y;
    }
    else if(face == FACE_BACK)
    {
        *uvX = 1 - x;
        *uvY = 1 - y;
    }
    else if(face == FACE_RIGHT)
    {
        *uvX = 1 - z;
        *uvY = 1 - y;
    }
    else if(face == FACE_LEFT)
    {
        *uvX = z;
        *uvY = 1 - y;
    }
    else if(face == FACE_UP)
    {
        *uvX = x;
        *uvY = z;
    }
    else if(face == FACE_DOWM)
    {
        *uvX = x;
        *uvY = 1 - z;
    }    
}

void vertexData_append_block_face(GLfloat *vertexData, int *vertexIndex, int x, int y, int z, Face face, int uvOffsetX, int uvOffsetY)
{
    for (size_t i = 0; i < 18; i+=3)
    {
        vertexData[(*vertexIndex)++] = vertecies[i + face * 18] + x;
        vertexData[(*vertexIndex)++] = vertecies[i + 1 + face * 18] + y;
        vertexData[(*vertexIndex)++] = vertecies[i + 2 + face * 18] + z;

        vertexData[(*vertexIndex)++] = faceBrightness[face];

        GLfloat uvX;
        GLfloat uvY;
        vertex_get_uv(face, vertecies[i + face * 18], vertecies[i + 1 + face * 18], vertecies[i + 2 + face * 18], &uvX, &uvY);

        vertexData[(*vertexIndex)++] = (uvX + uvOffsetX) / 8;
        vertexData[(*vertexIndex)++] = (uvY + uvOffsetY) / 8;
    }
}

void mesh_delete(Mesh *mesh)
{
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteVertexArrays(1, &mesh->vao);
    mesh->vertexCount = 0;
    mesh->vao = 0;
    mesh->vbo = 0;
}