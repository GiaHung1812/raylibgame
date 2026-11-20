#pragma once

#include <raylib.h>
#include <vector>
#include "collisions.h"

struct Player
{
    Vector3 position;
    Vector3 velocity;
    Vector3 colliderSize;
    float moveSpeed;
    float health;
    float eyeHeight;
    bool isGrounded;
    float bobTimer;
    float bobOffset;
};

void InitPlayer(Player &player, Vector3 startPos);
void UpdatePlayer(Player &player, Vector3 moveDirection, bool jumpPressed, float deltaTime, const std::vector<AABB> &obstacles);
Vector3 GetPlayerEyePosition(const Player &player);
AABB GetPlayerAABB(const Player &player);
