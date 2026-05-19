#include "chunkmatrix.h"

#include <stdlib.h>

void chunkmatrix_initialize(Chunkmatrix *chunkmatrix, int render_distance)
{
    chunkmatrix->render_distance = render_distance;
    chunkmatrix->grid_size = render_distance * 2 + 1;
    chunkmatrix->centerX = 0;
    chunkmatrix->centerZ = 0;
    
    chunkmatrix->chunks = malloc(chunkmatrix->grid_size * sizeof(Chunk*));

    for (size_t i = 0; i < chunkmatrix->grid_size; i++)
    {
        chunkmatrix->chunks[i] = malloc(chunkmatrix->grid_size * sizeof(Chunk));
    }
}

void chunkmatrix_shift_rigth(Chunkmatrix *chunkmatrix)
{
    for (size_t z = 0; z < chunkmatrix->grid_size; z++)
    {
        for (size_t x = 0; x < chunkmatrix->grid_size - 2; x++)
        {
            chunkmatrix->chunks[x + 1][z] = chunkmatrix->chunks[x][z];
        }
    }

    for (size_t z = 0; z < chunkmatrix->grid_size; z++)
    {
        chunkmatrix->chunks[0][z] = chunkmatrix->chunks[chunkmatrix->grid_size - 1][z];
        chunkmatrix->chunks[chunkmatrix->grid_size - 1][z] = chunkmatrix->chunks[chunkmatrix->grid_size - 2][z];
    }

    chunkmatrix->centerX++;
}

void chunkmatrix_generate_left(Chunkmatrix *chunkmatrix)
{
    for (size_t z = 0; z < chunkmatrix->grid_size; z++)
    {
        chunk_generate(chunkmatrix->chunks[0] + z, chunkmatrix->centerX - chunkmatrix->render_distance, chunkmatrix->centerZ + z - chunkmatrix->render_distance);
        chunkmatrix->chunks[0][z].neighbours.right = chunkmatrix->chunks[1] + z;
        chunkmatrix->chunks[1][z].neighbours.left = chunkmatrix->chunks[0] + z;
    }
}

void chunkmatrix_shift_left(Chunkmatrix *chunkmatrix)
{
    for (size_t z = 0; z < chunkmatrix->grid_size; z++)
    {
        for (size_t x = chunkmatrix->grid_size; x < chunkmatrix->grid_size - 2; x++)
        {
            chunkmatrix->chunks[x][z] = chunkmatrix->chunks[x][z];
        }
    }

    for (size_t z = 0; z < chunkmatrix->grid_size; z++)
    {
        chunkmatrix->chunks[0][z] = chunkmatrix->chunks[chunkmatrix->grid_size - 1][z];
        chunkmatrix->chunks[chunkmatrix->grid_size - 1][z] = chunkmatrix->chunks[chunkmatrix->grid_size - 2][z];
    }

    chunkmatrix->centerX++;
}