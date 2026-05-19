#ifndef CHUNKARRAY_H
#define CHUNKARRAY_H

#include "chunk.h"
#include <cglm/vec3.h>

typedef struct Chunkarray
{
    Chunk *chunks;
    int renderDistance;
    int gridSize;
    int chunkCount;
    int centerX;
    int centerZ;
    Chunk *center;
}
Chunkarray;

typedef struct RayHit
{
    Chunk *hitChunk;
    int hitBlockPos[3];

    Chunk *targetChunk;
    int targetBlockPos[3];
}
RayHit;

void chunkarray_initialize(Chunkarray *chunkarray, int renderDistance);
void chunkarray_generate_chunks(Chunkarray *chunkarray, int positionX, int positionZ);
void chunkarray_load_chunk(Chunkarray *chunkarray, int index);
void chunkarray_generate_chunk(Chunkarray *chunkarray, int index, int positionX, int positionZ);
void chunkarray_unload_chunk(Chunkarray *chunkarray, int index);
void chunkarray_load_chunks(Chunkarray *chunkarray);
void chunkarray_replace_chunk_out_of_render(Chunkarray *chunkarray, int index, float positionX, float positionZ);
bool chunkarray_raycast(Chunkarray *chunkarray, vec3 position, vec3 direction, float distance, RayHit *rayHit);

#endif