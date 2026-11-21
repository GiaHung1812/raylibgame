#pragma once

#include <vector>
#include "collisions.h"

struct World
{
    std::vector<AABB> obstacles;
};

void InitWorld(World &world);
void DrawWorld(const World &world);
bool LoadDungeonFromImage(World &world, const char *imagePath, Vector3 &playerSpawnPos);