#include "chunkarray.h"

#include <stdio.h>
#include <stdlib.h> 
#include <math.h>

void chunkarray_initialize(Chunkarray *chunkarray, int renderDistance)
{
    chunkarray->renderDistance = renderDistance;
    chunkarray->gridSize = renderDistance * 2 + 1;
    chunkarray->chunkCount = chunkarray->gridSize * chunkarray->gridSize;
    chunkarray->chunks = malloc(sizeof(Chunk) * chunkarray->chunkCount);
}

void chunkarray_generate_chunks(Chunkarray *chunkarray, int positionX, int positionZ)
{
    chunkarray->centerX = positionX;
    chunkarray->centerZ = positionZ;

    for (size_t i = 0; i < chunkarray->chunkCount; i++)
    {
        chunk_generate(chunkarray->chunks + i, i % chunkarray->gridSize - chunkarray->renderDistance, i / chunkarray->gridSize - chunkarray->renderDistance);
        if(i % chunkarray->gridSize - chunkarray->renderDistance == 0 && i / chunkarray->gridSize - chunkarray->renderDistance == 0)
        {
            chunkarray->center = chunkarray->chunks + i;
        }
        
        if(i / chunkarray->gridSize != 0)
        {
            chunkarray->chunks[i].neighbours.back = chunkarray->chunks + i - chunkarray->gridSize;
            chunkarray->chunks[i].neighbours.back->neighbours.front = chunkarray->chunks + i;
        }

        if(i % chunkarray->gridSize != 0)
        {
            chunkarray->chunks[i].neighbours.left = chunkarray->chunks + i - 1;
            chunkarray->chunks[i].neighbours.left->neighbours.right = chunkarray->chunks + i;
        }
    }
}

void chunkarray_load_chunks(Chunkarray *chunkarray)
{
    for (size_t i = 0; i < chunkarray->chunkCount; i++)
    {
        chunk_load(chunkarray->chunks + i);
    }
}

void chunkarray_replace_chunk_out_of_render(Chunkarray *chunkarray, int index, float positionX, float positionZ)
{
    int chunkPosX = (int)floorf(positionX / CHUNK_SIZE_X);
    int distanceX = (chunkPosX - chunkarray->chunks[index].positionX);

    int chunkPosZ = (int)floorf(positionZ / CHUNK_SIZE_Z);
    int distanceZ = (chunkPosZ - chunkarray->chunks[index].positionZ);

    if(distanceX > chunkarray->renderDistance)
    {
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkPosX + chunkarray->renderDistance, chunkarray->chunks[index].positionZ);
        chunkarray_load_chunk(chunkarray, index);
    }
    else if(distanceX < -chunkarray->renderDistance)
    {       
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkPosX - chunkarray->renderDistance, chunkarray->chunks[index].positionZ);
        chunkarray_load_chunk(chunkarray, index);
    }

    if(distanceZ > chunkarray->renderDistance)
    {
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkarray->chunks[index].positionX, chunkPosZ + chunkarray->renderDistance);
        chunkarray_load_chunk(chunkarray, index);
    }
    else if(distanceZ < -chunkarray->renderDistance)
    {       
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkarray->chunks[index].positionX, chunkPosZ - chunkarray->renderDistance);
        chunkarray_load_chunk(chunkarray, index);
    }
}

void chunkarray_load_chunk(Chunkarray *chunkarray, int index)
{
    chunkNeighbours_reload_meshes(&chunkarray->chunks[index].neighbours);
    chunk_load(chunkarray->chunks + index);
}

void chunkarray_generate_chunk(Chunkarray *chunkarray, int index, int positionX, int positionZ)
{
    chunk_generate(chunkarray->chunks + index, positionX, positionZ);

    for (size_t i = 0; i < chunkarray->chunkCount; i++)
    {
        if(chunkarray->chunks[i].positionZ == positionZ)
        {
            if(chunkarray->chunks[i].positionX == positionX - 1)
            {
                chunkarray->chunks[index].neighbours.left = chunkarray->chunks + i;
                chunkarray->chunks[i].neighbours.right = chunkarray->chunks + index; 
            }
            else if(chunkarray->chunks[i].positionX == positionX + 1)
            {
                chunkarray->chunks[index].neighbours.right = chunkarray->chunks + i;
                chunkarray->chunks[i].neighbours.left = chunkarray->chunks + index;
            }
        }
        else if(chunkarray->chunks[i].positionX == positionX)
        {
            if(chunkarray->chunks[i].positionZ == positionZ - 1)
            {
                chunkarray->chunks[index].neighbours.back = chunkarray->chunks + i;
                chunkarray->chunks[i].neighbours.front = chunkarray->chunks + index; 
            }
            else if(chunkarray->chunks[i].positionZ == positionZ + 1)
            {
                chunkarray->chunks[index].neighbours.front = chunkarray->chunks + i;
                chunkarray->chunks[i].neighbours.back = chunkarray->chunks + index;
            }
        }
    }
    
}

void chunkarray_unload_chunk(Chunkarray *chunkarray, int index)
{
    chunk_unload(chunkarray->chunks + index);
}

Chunk *chunkarray_get_chunk_at(Chunkarray *chunkarray, int positionX, int positionZ)
{
    for (size_t i = 0; i < chunkarray->chunkCount; i++)
    {
        if(chunkarray->chunks[i].positionX == positionX && chunkarray->chunks[i].positionZ == positionZ)
        {
            return chunkarray->chunks + i;
        }
    }
    
    return NULL;
}

bool chunkarray_raycast(Chunkarray *chunkarray, vec3 originalPosition, vec3 direction, float distance, RayHit *rayHit)
{
    int lastBlockPos[3];
    float stepSize = 0.05f;
    float originalDistance = distance;
    vec3 position;
    glm_vec3_copy(originalPosition, position);
    while (distance > 0)
    {
        distance -= stepSize;
        vec3 temp;
        glm_vec3_scale(direction, stepSize, temp);
        glm_vec3_add(position, temp, position);

        int chunkposX = (int)floorf(position[0] / CHUNK_SIZE_X);
        int chunkposZ = (int)floorf(position[2] / CHUNK_SIZE_Z);
  
        float posInChunkX = position[0] - chunkposX * CHUNK_SIZE_X;
        float posInChunkY = position[1];
        float posInChunkZ = position[2] - chunkposZ * CHUNK_SIZE_Z;

        int blockposX = (int)floorf(posInChunkX);
        int blockposY = (int)floorf(posInChunkY);
        int blockposZ = (int)floorf(posInChunkZ);

        if(distance + stepSize == originalDistance)
        {
            lastBlockPos[0] = blockposX;
            lastBlockPos[1] = blockposY;
            lastBlockPos[2] = blockposZ;
        }

        Chunk *chunk = chunkarray_get_chunk_at(chunkarray, chunkposX, chunkposZ);

        if(!chunk) return false;

        if(chunk->data[blockposX][blockposY][blockposZ] != BLOCK_AIR)
        {
            rayHit->hitChunk = chunk;
            rayHit->hitBlockPos[0] = blockposX;
            rayHit->hitBlockPos[1] = blockposY;
            rayHit->hitBlockPos[2] = blockposZ;

            rayHit->targetBlockPos[0] = lastBlockPos[0];
            rayHit->targetBlockPos[1] = lastBlockPos[1];
            rayHit->targetBlockPos[2] = lastBlockPos[2];

            if(lastBlockPos[0] != blockposX)
            {                
                if(lastBlockPos[0] == CHUNK_SIZE_X - 1 && blockposX == 0)
                {
                    rayHit->targetChunk = chunk->neighbours.left;
                }
                else if(lastBlockPos[0] == 0 && blockposX == CHUNK_SIZE_X - 1)
                {
                    rayHit->targetChunk = chunk->neighbours.right;
                }
                else
                {
                    rayHit->targetChunk = chunk;
                }               
            }
            else if(lastBlockPos[1] != blockposY)
            {
                rayHit->targetChunk = chunk;
            }
            else if(lastBlockPos[2] != blockposZ)
            {
                if(lastBlockPos[2] == CHUNK_SIZE_Z - 1 && blockposZ == 0)
                {
                    rayHit->targetChunk = chunk->neighbours.back;
                }
                else if(lastBlockPos[2] == 0 && blockposZ == CHUNK_SIZE_Z - 1)
                {
                    rayHit->targetChunk = chunk->neighbours.front;
                }
                else
                {
                    rayHit->targetChunk = chunk;
                }
            }
            else
            {
                rayHit->targetChunk = chunk;
            }

            return true;
        }
        else
        {
            lastBlockPos[0] = blockposX;
            lastBlockPos[1] = blockposY;
            lastBlockPos[2] = blockposZ;
        }
    }

    return false;
}