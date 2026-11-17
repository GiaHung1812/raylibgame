#pragma once
#include <raylib.h>
#include <vector>

using CollisonList = std::vector<struct AABB>;

struct AABB
{
    Vector3 min;
    Vector3 max;
};

bool CheckAABBCollision(const AABB &a, const AABB &b);