#include "collisions.h"

bool CheckAABBCollision(const AABB &a, const AABB &b)
{
    return !(a.max.x < b.min.x || b.max.x < a.min.x ||
             a.max.y < b.min.y || b.max.y < a.min.y ||
             a.max.z < b.min.z || b.max.z < a.min.z);
}

bool CheckWorldCollision(const AABB &box, const CollisonList &obstacles)
{
    for (const auto &obstacle : obstacles)
    {
        if (CheckAABBCollision(box, obstacle))
        {
            return true;
        }
    }
    return false;
}