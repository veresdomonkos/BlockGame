#ifndef PLAYER_H
#define PLAYER_H

#include <cglm/vec3.h>

typedef struct Player
{
    vec3 position;
    float yaw;
    float pitch;
    vec3 forward;
}
Player;

void player_rotate(Player *player, vec2 mouseOffset);
void player_move(Player *player, vec3 directions, double deltaTime);

#endif