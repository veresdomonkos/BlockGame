#include "chunk.h"
#include <stdlib.h>
#include <stdio.h>

#define STB_PERLIN_IMPLEMENTATION
#include "stb/stb_perlin.h"

float octave_noise(int x, int z, int octaves, float persistence, float base_frequency, float base_amplitude, int seed)
{
    float amplitude = base_amplitude;
    float frequency = base_frequency;
    float noise = 0.0f;
    float amplitudeSum = 0.0f;

    for (int i = 0; i < octaves; i++)
    {
        amplitudeSum += amplitude;
        noise += amplitude * stb_perlin_noise3_seed(frequency * x, 0, frequency * z, 0, 0, 0, seed);
        frequency *= 2.0f;
        amplitude *= persistence;
    }

    return noise / amplitudeSum;
}

float octave_noise3(int x, int y, int z, int octaves, float persistence, float base_frequency, float base_amplitude, int seed)
{
    float amplitude = base_amplitude;
    float frequency = base_frequency;
    float noise = 0.0f;
    float amplitudeSum = 0.0f;

    for (int i = 0; i < octaves; i++)
    {
        amplitudeSum += amplitude;
        noise += amplitude * stb_perlin_noise3_seed(frequency * x, frequency * y, frequency * z, 0, 0, 0, seed);
        frequency *= 2.0f;
        amplitude *= persistence;
    }

    return noise / amplitudeSum;
}

float height_fade(int y, int upper)
{
    if(y < 10)
    {
        return (float)y / 10;
    }

    return (float)(upper - y) / (upper);
}

void chunk_generate(Chunk *chunk, int positionX, int positionZ)
{
    chunk->positionX = positionX;
    chunk->positionZ = positionZ;

    for (size_t x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (size_t z = 0; z < CHUNK_SIZE_Z; z++)
        {
            float noise = octave_noise((1000 + positionX) * CHUNK_SIZE_X + x, (1000 + positionZ) * CHUNK_SIZE_Z + z, 5, 0.5f, 0.005f, 1, 0);
            noise = noise * (CHUNK_SIZE_Y / 2) + (CHUNK_SIZE_Y / 2);
            int upper = (int)noise;
            //printf("%f\n", upper);
            for (size_t y = 0; y < CHUNK_SIZE_Y; y++)
            {
                if(y > 0 && y <= upper)
                {
                    float caveNoise = octave_noise3((1000 + positionX) * CHUNK_SIZE_X + x, y, (1000 + positionZ) * CHUNK_SIZE_Z + z, 3, 0.4f, 0.045f, 1, 0);
                    //printf("%f\n",caveNoise);
                    if(height_fade(y, upper*2.0f) * caveNoise < -0.2f)
                    {
                        chunk->data[x][y][z] = BLOCK_AIR;
                        continue;
                    }
                }

                if(y == upper)
                {
                    chunk->data[x][y][z] = BLOCK_GRASS;
                }
                else if(y < upper)
                {
                    if(y == 0)
                    {
                        chunk->data[x][y][z] = BLOCK_BEDROCK;
                    }
                    else if(y > upper - 5)
                    {
                        chunk->data[x][y][z] = BLOCK_DIRT;
                    }
                    else
                    {
                        chunk->data[x][y][z] = BLOCK_STONE;
                    }                  
                }
                else
                {
                    chunk->data[x][y][z] = BLOCK_AIR;
                }
            }            
        }      
    } 
}


void blockface_get_uv(BlockType block, Face face, int *uvOffsetX, int *uvOffsetY)
{
    if(block == BLOCK_GRASS)
    {
        if(face == FACE_UP)
        {
            *uvOffsetX = 1;
            *uvOffsetY = 0;
        }
        else if(face == FACE_DOWM)
        {
            *uvOffsetX = 2;
            *uvOffsetY = 0;
        }
        else
        {          
            *uvOffsetX = 0;
            *uvOffsetY = 0;
        }
    }
    else if (block == BLOCK_DIRT)
    {
        *uvOffsetX = 2;
        *uvOffsetY = 0;
    }
    else if (block == BLOCK_BEDROCK)
    {
        *uvOffsetX = 4;
        *uvOffsetY = 0;
    }
    else if (block == BLOCK_STONE)
    {
        *uvOffsetX = 3;
        *uvOffsetY = 0;
    }
}

void chunk_upload_mesh(Chunk *chunk, int vertexIndex, GLfloat *vertexData)
{
    chunk->mesh.vertexCount = vertexIndex / 6;
    glGenVertexArrays(1, &chunk->mesh.vao);
    glBindVertexArray(chunk->mesh.vao);
    
    glGenBuffers(1, &chunk->mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexIndex * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    free(vertexData);
}


GLfloat* chunk_create_mesh(Chunk *chunk, int *vertexIndex)
{
    *vertexIndex = 0;
    int maxVertexCount = 256 * 36 * 40 * 2;

    GLfloat *vertexData = malloc(maxVertexCount * sizeof(GLfloat));
    
    for (size_t x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (size_t z = 0; z < CHUNK_SIZE_Z; z++)
        {
            for (size_t y = 0; y < CHUNK_SIZE_Y; y++)
            {
                if(chunk->data[x][y][z] != BLOCK_AIR)
                {
                    int worlddPositionX = x + chunk->positionX * CHUNK_SIZE_X;
                    int worldPositionZ = z + chunk->positionZ * CHUNK_SIZE_Z;
                    int uvOffsetX;
                    int uvOffsetY;

                    bool createRightFace = (x == CHUNK_SIZE_X - 1 && (!chunk->neighbours.right || chunk->neighbours.right->data[0][y][z] == BLOCK_AIR)) || (x != CHUNK_SIZE_X - 1 && chunk->data[x + 1][y][z] == BLOCK_AIR);
                    bool createLeftFace = (x == 0 && (!chunk->neighbours.left || chunk->neighbours.left->data[CHUNK_SIZE_X - 1][y][z] == BLOCK_AIR)) || (x != 0 && chunk->data[x - 1][y][z] == BLOCK_AIR);

                    bool createFrontFace = (z == CHUNK_SIZE_Z - 1 && (!chunk->neighbours.front || chunk->neighbours.front->data[x][y][0] == BLOCK_AIR)) || (z != CHUNK_SIZE_Z - 1 && chunk->data[x][y][z + 1] == BLOCK_AIR);
                    bool createBackFace = (z == 0 && (!chunk->neighbours.back || chunk->neighbours.back->data[x][y][CHUNK_SIZE_Z - 1] == BLOCK_AIR)) || (z != 0 && chunk->data[x][y][z - 1] == BLOCK_AIR);

                    if(createRightFace)
                    {
                        blockface_get_uv(chunk->data[x][y][z], FACE_RIGHT, &uvOffsetX, &uvOffsetY);
                        vertexData_append_block_face(vertexData, vertexIndex, worlddPositionX, y, worldPositionZ, FACE_RIGHT, uvOffsetX, uvOffsetY);
                    }
                    if(createLeftFace)
                    {
                        blockface_get_uv(chunk->data[x][y][z], FACE_LEFT, &uvOffsetX, &uvOffsetY);
                        vertexData_append_block_face(vertexData, vertexIndex, worlddPositionX, y, worldPositionZ, FACE_LEFT, uvOffsetX, uvOffsetY);
                    }
                    if(y == CHUNK_SIZE_Y - 1 || chunk->data[x][y + 1][z] == BLOCK_AIR)
                    {
                        blockface_get_uv(chunk->data[x][y][z], FACE_UP, &uvOffsetX, &uvOffsetY);
                        vertexData_append_block_face(vertexData, vertexIndex, worlddPositionX, y, worldPositionZ, FACE_UP, uvOffsetX, uvOffsetY);
                    }
                    if(y == 0 || chunk->data[x][y - 1][z] == BLOCK_AIR)
                    {
                        blockface_get_uv(chunk->data[x][y][z], FACE_DOWM, &uvOffsetX, &uvOffsetY);
                        vertexData_append_block_face(vertexData, vertexIndex, worlddPositionX, y, worldPositionZ, FACE_DOWM, uvOffsetX, uvOffsetY);
                    }
                    if(createFrontFace)
                    {
                        blockface_get_uv(chunk->data[x][y][z], FACE_FRONT, &uvOffsetX, &uvOffsetY);
                        vertexData_append_block_face(vertexData, vertexIndex, worlddPositionX, y, worldPositionZ, FACE_FRONT, uvOffsetX, uvOffsetY);
                    }
                    if(createBackFace)
                    {
                        blockface_get_uv(chunk->data[x][y][z], FACE_BACK, &uvOffsetX, &uvOffsetY);
                        vertexData_append_block_face(vertexData, vertexIndex, worlddPositionX, y, worldPositionZ, FACE_BACK, uvOffsetX, uvOffsetY);
                    }
                }
            }            
        }
    }

   return vertexData;
}

void chunk_load(Chunk *chunk)
{
    chunk->isLoaded = true;
    int vertexIndex;
    GLfloat *vertexData = chunk_create_mesh(chunk, &vertexIndex);
    chunk_upload_mesh(chunk, vertexIndex, vertexData);
}

void chunk_reload_mesh(Chunk *chunk)
{
    mesh_delete(&chunk->mesh);
    int vertexIndex;
    GLfloat *vertexData = chunk_create_mesh(chunk, &vertexIndex);
    chunk_upload_mesh(chunk, vertexIndex, vertexData);
}

void chunkNeighbours_reload_meshes(ChunkNeighbours *neighbours)
{
    if(neighbours->back)
    {
        chunk_reload_mesh(neighbours->back);
    }

    if(neighbours->front)
    {
        chunk_reload_mesh(neighbours->front);
    }

    if(neighbours->left)
    {
        chunk_reload_mesh(neighbours->left);
    }

    if(neighbours->right)
    {
        chunk_reload_mesh(neighbours->right);
    }
}

void chunk_place_block(Chunk *chunk, int x, int y, int z, BlockType block)
{
    if(x > CHUNK_SIZE_X - 1 || x < 0 || y > CHUNK_SIZE_Y - 1 || y < 0 || z > CHUNK_SIZE_Z - 1 || z < 0)
    {
        return;
    }

    chunk->data[x][y][z] = block;
    chunk_reload_mesh(chunk);

    if(x == 0)
    {
        chunk_reload_mesh(chunk->neighbours.left);
    }
    else if (x == CHUNK_SIZE_X - 1)
    {
        chunk_reload_mesh(chunk->neighbours.right);
    }

    if(z == 0)
    {
        chunk_reload_mesh(chunk->neighbours.back);
    }
    else if (z == CHUNK_SIZE_Z - 1)
    {
        chunk_reload_mesh(chunk->neighbours.front);
    }
}

void chunk_unload(Chunk *chunk)
{
    chunk->isLoaded = false;
    mesh_delete(&chunk->mesh);

    Chunk *backNeighbour = chunk->neighbours.back;
    Chunk *frontNeighbour = chunk->neighbours.front;
    Chunk *leftNeighbour = chunk->neighbours.left;
    Chunk *rightNeighbour = chunk->neighbours.right;
    if(backNeighbour)
    {
        chunk->neighbours.back = NULL;
        backNeighbour->neighbours.front = NULL;
        chunk_reload_mesh(backNeighbour);
    }
    if(frontNeighbour)
    {
        chunk->neighbours.front = NULL;
        frontNeighbour->neighbours.back = NULL;
        chunk_reload_mesh(frontNeighbour);
    }
    if(leftNeighbour)
    {
        chunk->neighbours.left = NULL;
        leftNeighbour->neighbours.right = NULL;
        chunk_reload_mesh(leftNeighbour);
    }
    if(rightNeighbour)
    {
        chunk->neighbours.right = NULL;
        rightNeighbour->neighbours.left = NULL;
        chunk_reload_mesh(rightNeighbour);
    }
}