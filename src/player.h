#pragma once
#include <raylib.h>

struct Player
{
    Vector3 position;
    Vector3 velocity;
    float moveSpeed;
    float health;
    float eyeHeight;
};

void InitPlayer(Player &player, Vector3 startPos);
void UpdatePlayer(Player &player, Vector3 moveDirection, float deltaTime);
Vector3 GetPlayerEyePosition(const Player &player);
