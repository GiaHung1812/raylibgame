#pragma once
#include <raylib.h>

struct AABB
{
    Vector3 min;
    Vector3 max;
};

bool CheckAABBCollision(const AABB &a, const AABB &b);