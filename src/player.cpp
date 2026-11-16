#include "player.h"
#include <raymath.h>

void InitPlayer(Player &player, Vector3 startPos)
{
    player.position = startPos;
    player.velocity = {0.0f, 0.0f, 0.0f};
    player.moveSpeed = 5.0f;
    player.health = 100.0f;
    player.eyeHeight = 1.6f;
}

void UpdatePlayer(Player &player, Vector3 moveDirection, float deltaTime)
{
    player.position.x += moveDirection.x * player.moveSpeed * deltaTime;
    player.position.z += moveDirection.z * player.moveSpeed * deltaTime;
}

Vector3 GetPlayerEyePosition(const Player &player)
{
    return {player.position.x, player.position.y + player.eyeHeight, player.position.z};
}
