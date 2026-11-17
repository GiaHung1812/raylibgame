#pragma once
#include <raylib.h>
#include "collisions.h"

struct Player
{
    Vector3 position;
    Vector3 velocity;
    Vector3 colliderSize;
    float moveSpeed;
    float health;
    float eyeHeight;
};

void InitPlayer(Player &player, Vector3 startPos);
void UpdatePlayer(Player &player, Vector3 moveDirection, float deltaTime);
Vector3 GetPlayerEyePosition(const Player &player);
AABB GetPlayerAABB(const Player &player);