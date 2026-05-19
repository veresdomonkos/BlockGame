#include "player.h"

void player_rotate(Player *player, vec2 mouseOffset)
{
    player->yaw += mouseOffset[0] * 0.1f;
    player->pitch += mouseOffset[1] * 0.1f;

    player->pitch = glm_clamp(player->pitch, -89, 89);
    
    float yawRadian = glm_rad(player->yaw);
    float pitchRadian = glm_rad(player->pitch);

    player->forward[0] = sinf(yawRadian) * cosf(pitchRadian);
    player->forward[1] = sinf(pitchRadian);
    player->forward[2] = -cosf(yawRadian) * cosf(pitchRadian);

    glm_normalize(player->forward);
}

void player_move(Player *player, vec3 directions, double deltaTime)
{
    vec3 forward2;
    forward2[0] = player->forward[0];
    forward2[1] = 0;
    forward2[2] = player->forward[2];
    glm_normalize(forward2);

    vec3 right = {-forward2[2], 0, forward2[0]};

    glm_vec3_scale(forward2, directions[0] * deltaTime * 15, forward2);
    glm_vec3_scale(right, directions[2] * deltaTime * 15, right);

    glm_vec3_add(forward2, player->position, player->position);
    glm_vec3_add(right, player->position, player->position);
    player->position[1] += directions[1] * deltaTime * 15;
}
