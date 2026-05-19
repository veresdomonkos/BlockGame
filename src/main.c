#include <stdio.h>
#include "render.h"
#include "input.h"
#include "player.h"
#include "chunkarray.h"
#include "queue.h"

void handle_raycast(Chunkarray *chunkarray, Player *player);
void handle_chunk_rendering(Chunkarray *chunkarray, Player *player);

int main()
{
    if(!initialize_window())
    {
        return -1;
    }

    configure_render();
    initialize_shaders();

    printf("%d", -1 % 8);

    Chunkarray chunkarray;
    chunkarray_initialize(&chunkarray, RENDER_DISTANCE);
    //chunkarray_generate_chunks(&chunkarray, 0, 0);
    //chunkarray_load_chunks(&chunkarray);

    MeshDataQueue queue;
    queue_init(&queue);

    Player player = 
    {
        .position[0] = 0,
        .position[1] = 160,
        .position[2] = 4,
        .yaw = 0,
        .pitch = 0,
    };

    WorkerThreadArgs args = {.chunkarray = &chunkarray, .player_x = player.position, .player_z = player.position + 2, .queue = &queue};

    pthread_t worker;
    pthread_create(&worker, NULL, worker_thread, &args);

    double lastTime = glfwGetTime();
    double deltaTime;

    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        //printf("fps: %f\n",1/deltaTime);

        input_update_mouse_offset(window, SCREEN_WIDTH, SCREEN_HEIGHT);
        input_update_directions(window);
        input_update_mouse_buttons(window);

        player_rotate(&player, mouseOffset);
        player_move(&player, directions, deltaTime);

        update_view(player.position, player.forward);

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
        MeshDataNode node;

        if(queue_try_pop(&queue, &node))
        {
            chunk_upload_mesh(node.chunk, node.vertexIndex, node.data);
            node.chunk->isLoaded = true;
        }

        handle_chunk_rendering(&chunkarray, &player);
        handle_raycast(&chunkarray, &player);
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    stop_flag = 1;
    pthread_join(worker, NULL);
    queue_destroy(&queue);
    return 0;
}

void handle_chunk_rendering(Chunkarray *chunkarray, Player *player)
{
    for (size_t i = 0; i < chunkarray->chunkCount; i++)
    {
        //chunkarray_replace_chunk_out_of_render(chunkarray, i, player->position[0], player->position[2]);

        if(chunkarray->chunks[i].isLoaded)
        {
            render_mesh(&chunkarray->chunks[i].mesh);
        }
    }
}

void handle_raycast(Chunkarray *chunkarray, Player *player)
{
    RayHit hit;
    if(chunkarray_raycast(chunkarray, player->position, player->forward, 3, &hit))
    {
        render_update_selection(hit.hitBlockPos[0] + hit.hitChunk->positionX * CHUNK_SIZE_X, hit.hitBlockPos[1], hit.hitBlockPos[2] + hit.hitChunk->positionZ * CHUNK_SIZE_Z);
        if(mouseButtons[1])
        {
            chunk_place_block(hit.targetChunk, hit.targetBlockPos[0], hit.targetBlockPos[1], hit.targetBlockPos[2], BLOCK_STONE);
        }
        else if(mouseButtons[0])
        {
            chunk_place_block(hit.hitChunk, hit.hitBlockPos[0], hit.hitBlockPos[1], hit.hitBlockPos[2], BLOCK_AIR);
        }
    }
    else
    {
        render_update_selection(0, -10, 0);
    }
}