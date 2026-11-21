#include "world.h"

constexpr float BLOCK_SIZE = 2.0f;
constexpr float WALL_HEIGHT = 2.5f; // Dungeon height

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
    // Example obstacles (placed in front of and around the player spawn)
    world.obstacles.clear();
    world.obstacles.push_back(CreateWorldAABB({0.0f, 0.0f, -3.0f}, {2.0f, 1.0f, 1.5f})); // Low step crate (1m tall)
    world.obstacles.push_back(CreateWorldAABB({0.0f, 0.0f, -5.0f}, {2.0f, 2.0f, 2.0f})); // Main box (2m tall)
    world.obstacles.push_back(CreateWorldAABB({4.0f, 0.0f, -8.0f}, {2.0f, 3.0f, 2.0f}));
    world.obstacles.push_back(CreateWorldAABB({-4.0f, 0.0f, -8.0f}, {2.0f, 2.0f, 2.0f}));
    world.obstacles.push_back(CreateWorldAABB({0.0f, 0.0f, 5.0f}, {2.0f, 2.0f, 2.0f})); // Behind
}

bool LoadDungeonFromImage(World &world, const char *imagePath, Vector3 &playerSpawnPos)
{
    Image mapImage = LoadImage(imagePath);
    if (mapImage.data == nullptr)
    {
        return false;
    }
    world.obstacles.clear();

    float halfWidth = (float)mapImage.width * 0.5f;
    float halfHeight = (float)mapImage.height * 0.5f;
    playerSpawnPos = {0.0f, 0.0f, 0.0f};
    for (int y = 0; y < mapImage.height; y++)
    {
        for (int x = 0; x < mapImage.width; x++)
        {
            Color pixelColor = GetImageColor(mapImage, x, y);
            // Change pixel to 3D world
            float worldX = (x - halfWidth) * BLOCK_SIZE;
            float worldZ = (y - halfHeight) * BLOCK_SIZE;

            if (pixelColor.r == 0 && pixelColor.g == 0 && pixelColor.b == 0 && pixelColor.a > 0)
            {
                world.obstacles.push_back(CreateWorldAABB({worldX, 0.0f, worldZ}, {BLOCK_SIZE, WALL_HEIGHT, BLOCK_SIZE}));
            }
            else if (pixelColor.g > 200 && pixelColor.r < 50 && pixelColor.b < 50)
            {
                playerSpawnPos = {worldX, 0.0f, worldZ};
            }
        }
    }
    UnloadImage(mapImage);
    return true;
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
        DrawCubeWires(position, size.x, size.y, size.z, DARKGRAY);
    }
}
