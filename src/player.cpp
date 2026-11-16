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
    // Normalize moveDirection to avoid faster diagonal speed
    if (Vector3Length(moveDirection) > 0.0f)
    {
        moveDirection = Vector3Normalize(moveDirection);
    }

    // Calculate horizontal velocity using movement direction and speed
    player.velocity.x = moveDirection.x * player.moveSpeed;
    player.velocity.z = moveDirection.z * player.moveSpeed;

    // Update position using velocity
    player.position.x += player.velocity.x * deltaTime;
    player.position.y += player.velocity.y * deltaTime;
    player.position.z += player.velocity.z * deltaTime;
}

Vector3 GetPlayerEyePosition(const Player &player)
{
    return {player.position.x, player.position.y + player.eyeHeight, player.position.z};
}
