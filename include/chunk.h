#ifndef CHUNK_H
#define CHUNK_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "mesh.h"
#include <stdbool.h>

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 256
#define CHUNK_SIZE_Z 16

typedef enum BlockType
{
    BLOCK_AIR,
    BLOCK_DIRT,
    BLOCK_GRASS,
    BLOCK_BEDROCK,
    BLOCK_STONE
}
BlockType;

struct Chunk;

typedef struct ChunkNeighbours
{
    struct Chunk *front;
    struct Chunk *back;
    struct Chunk *right;
    struct Chunk *left;
}
ChunkNeighbours;

typedef struct Chunk
{
    unsigned char data[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
    Mesh mesh;
    int positionX;
    int positionZ;
    bool isLoaded;
    ChunkNeighbours neighbours;
}
Chunk;

void chunk_generate(Chunk *chunk, int positionX, int positionZ);
GLfloat* chunk_create_mesh(Chunk *chunk, int *vertexIndex);
void chunk_reload_mesh(Chunk *chunk);
void chunk_load(Chunk *chunk);
void chunk_unload(Chunk *chunk);
void chunkNeighbours_reload_meshes(ChunkNeighbours *neighbours);
void chunk_place_block(Chunk *chunk, int x, int y, int z, BlockType block);
void chunk_upload_mesh(Chunk *chunk, int vertexIndex, GLfloat *vertexData);

#endif