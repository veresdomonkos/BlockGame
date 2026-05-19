#ifndef CHUNKMATRIX_H
#define CHUNKMATRIX_H

#include "chunk.h"

typedef struct Chunkmatrix
{
    Chunk **chunks;
    int grid_size;
    int render_distance;
    int centerX;
    int centerZ;
}
Chunkmatrix;

void chunkmatrix_initialize(Chunkmatrix *chunkmatrix, int render_distance);
void chunkmatrix_shift_rigth(Chunkmatrix *chunkmatrix);

#endif