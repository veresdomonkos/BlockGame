#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <Windows.h>
#include "chunkarray.h"
#include <stdio.h>

volatile int stop_flag = 0;

typedef struct MeshDataNode {
    GLfloat *data;
    int vertexIndex;
    Chunk* chunk;
    struct MeshDataNode *next;
} MeshDataNode;

// Thread-safe queue structure
typedef struct {
    MeshDataNode *front;
    MeshDataNode *rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} MeshDataQueue;

typedef struct WorkerThreadArgs
{
    MeshDataQueue *queue;
    Chunkarray *chunkarray;
    float *player_x;
    float *player_z;
} WorkerThreadArgs;

// Initialize the queue
void queue_init(MeshDataQueue *queue) {
    queue->front = queue->rear = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

// Push data onto the queue
void queue_push(MeshDataQueue *queue, GLfloat *data, int vertexIndex, Chunk *chunk) {
    MeshDataNode *node = (MeshDataNode*)malloc(sizeof(MeshDataNode));
    node->data = data;
    node->vertexIndex = vertexIndex;
    node->chunk = chunk;
    node->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    if (queue->rear) {
        queue->rear->next = node;
    } else {
        queue->front = node;
    }
    queue->rear = node;
    pthread_cond_signal(&queue->cond); // Notify waiting threads
    pthread_mutex_unlock(&queue->mutex);
}

// Pop data from the queue (blocking)
GLfloat *queue_pop(MeshDataQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (!queue->front) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    MeshDataNode *node = queue->front;
    GLfloat *data = node->data;
    queue->front = node->next;
    if (!queue->front) {
        queue->rear = NULL;
    }
    free(node);
    pthread_mutex_unlock(&queue->mutex);

    return data;
}

bool queue_try_pop(MeshDataQueue *queue, MeshDataNode *node) {
    pthread_mutex_lock(&queue->mutex);

    if (!queue->front) {
        pthread_mutex_unlock(&queue->mutex);
        return false; // Return NULL if the queue is empty
    }

    *node = *queue->front;
    free(queue->front);

    queue->front = node->next;
    if (!queue->front) {
        queue->rear = NULL;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    return true;
}

// Clean up the queue
void queue_destroy(MeshDataQueue *queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}

void chunk_reload_meshM(Chunk *chunk, MeshDataQueue *queue)
{
    if(!chunk) return;

    chunk->isLoaded = false;
    mesh_delete(&chunk->mesh);

    int vertexIndex;
    GLfloat *data = chunk_create_mesh(chunk, &vertexIndex);

    queue_push(queue, data, vertexIndex, chunk);
}

void chunkNeighbours_reload_meshesM(ChunkNeighbours *neighbours, MeshDataQueue *queue)
{
    chunk_reload_meshM(neighbours->back, queue);
    chunk_reload_meshM(neighbours->front, queue);
    chunk_reload_meshM(neighbours->left, queue);
    chunk_reload_meshM(neighbours->right, queue);
}

void chunkarray_replace_chunk_out_of_renderM(WorkerThreadArgs *args, int index)
{
    Chunkarray *chunkarray = args->chunkarray;

    int chunkPosX = (int)floorf(*args->player_x / CHUNK_SIZE_X);
    int distanceX = (chunkPosX - chunkarray->chunks[index].positionX);

    int chunkPosZ = (int)floorf(*args->player_z / CHUNK_SIZE_Z);
    int distanceZ = (chunkPosZ - chunkarray->chunks[index].positionZ);

    int vertexIndex;
    GLfloat *data;

    if(distanceX > chunkarray->renderDistance)
    {
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkPosX + chunkarray->renderDistance, chunkarray->chunks[index].positionZ);
        chunkNeighbours_reload_meshesM(&chunkarray->chunks[index].neighbours, args->queue);

        data = chunk_create_mesh(chunkarray->chunks + index, &vertexIndex);
        queue_push(args->queue, data, vertexIndex, chunkarray->chunks + index);
    }
    else if(distanceX < -chunkarray->renderDistance)
    {       
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkPosX - chunkarray->renderDistance, chunkarray->chunks[index].positionZ);
        chunkNeighbours_reload_meshesM(&chunkarray->chunks[index].neighbours, args->queue);

        data = chunk_create_mesh(chunkarray->chunks + index, &vertexIndex);
        queue_push(args->queue, data, vertexIndex, chunkarray->chunks + index);
    }

    if(distanceZ > chunkarray->renderDistance)
    {
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkarray->chunks[index].positionX, chunkPosZ + chunkarray->renderDistance);
        chunkNeighbours_reload_meshesM(&chunkarray->chunks[index].neighbours, args->queue);

        data = chunk_create_mesh(chunkarray->chunks + index, &vertexIndex);
        queue_push(args->queue, data, vertexIndex, chunkarray->chunks + index);
    }
    else if(distanceZ < -chunkarray->renderDistance)
    {       
        chunkarray_unload_chunk(chunkarray, index);
        chunkarray_generate_chunk(chunkarray, index, chunkarray->chunks[index].positionX, chunkPosZ - chunkarray->renderDistance);
        chunkNeighbours_reload_meshesM(&chunkarray->chunks[index].neighbours, args->queue);

        data = chunk_create_mesh(chunkarray->chunks + index, &vertexIndex);
        queue_push(args->queue, data, vertexIndex, chunkarray->chunks + index);
    }
}


// Function for worker thread
void *worker_thread(void *arg) {
    WorkerThreadArgs *args = (WorkerThreadArgs *)arg;
    MeshDataQueue *queue = args->queue;
    Chunkarray  *chunkarray = args->chunkarray;
    int vertexIndex;
    GLfloat *data;

    chunkarray_generate_chunks(args->chunkarray, 0, 0);

    for (int i = 0; i < chunkarray->chunkCount && !stop_flag; i++)
    {
        data = chunk_create_mesh(chunkarray->chunks + i, &vertexIndex);
        queue_push(queue, data, vertexIndex, chunkarray->chunks + i);
    }

    while (!stop_flag)
    {
        for (int i = 0; i < chunkarray->chunkCount; i++)
        {
            chunkarray_replace_chunk_out_of_renderM(args, i);
        }
    }

    return NULL;
}

#endif