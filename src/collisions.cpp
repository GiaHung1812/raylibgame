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

RayCollision RaycastWorld(Ray ray, float maxDistance, const CollisonList &obstacles)
{
    RayCollision closestHit{};

    for (const auto &obstacle : obstacles)
    {
        BoundingBox box = {obstacle.min, obstacle.max};
        RayCollision hit = GetRayCollisionBox(ray, box);
        if (hit.hit && hit.distance >= 0.0f && hit.distance <= maxDistance &&
            (!closestHit.hit || hit.distance < closestHit.distance))
        {
            closestHit = hit;
        }
    }

    return closestHit;
}