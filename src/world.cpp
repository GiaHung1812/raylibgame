#include "world.h"

static AABB CreateWorldAABB(const Vector3 &position, const Vector3 &size)
{
    AABB box;
    box.min = {
        position.x - size.x * 0.5f,
        position.y,
        position.z - size.z * 0.5f};
    box.max = {
        position.x + size.x * 0.5f,
        position.y + size.y,
        position.z + size.z * 0.5f};
    return box;
}

void InitWorld(World &world)
{
    // Example obstacles
    world.obstacles.clear();
    world.obstacles.push_back(CreateWorldAABB({0.0f, 0.0f, 5.0f}, {2.0f, 2.0f, 2.0f}));
    world.obstacles.push_back(CreateWorldAABB({3.0f, 0.0f, 10.0f}, {2.0f, 2.0f, 2.0f}));
    world.obstacles.push_back(CreateWorldAABB({-3.0f, 0.0f, 15.0f}, {2.0f, 2.0f, 2.0f}));
}

void DrawWorld(const World &world)
{
    for (const auto &obstacle : world.obstacles)
    {
        Vector3 size = {
            obstacle.max.x - obstacle.min.x,
            obstacle.max.y - obstacle.min.y,
            obstacle.max.z - obstacle.min.z};
        Vector3 position = {
            (obstacle.min.x + obstacle.max.x) * 0.5f,
            (obstacle.min.y + obstacle.max.y) * 0.5f,
            (obstacle.min.z + obstacle.max.z) * 0.5f};
        DrawCube(position, size.x, size.y, size.z, GRAY);
    }
}