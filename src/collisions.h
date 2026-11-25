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
bool CheckWorldCollision(const AABB &box, const CollisonList &obstacles);
RayCollision RaycastWorld(Ray ray, float maxDistance, const CollisonList &obstacles);