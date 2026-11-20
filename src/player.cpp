#include "player.h"
#include <raymath.h>

// Physics constants
constexpr float GRAVITY = -20.0f;       // Downward acceleration (m/s^2)
constexpr float JUMP_FORCE = 8.0f;      // Upward impulse on jump (m/s)
constexpr float BOB_FREQUENCY = 10.0f;  // Head bobbing speed (steps/sec)
constexpr float BOB_AMPLITUDE = 0.05f;  // Head bobbing vertical offset (meters)

void InitPlayer(Player &player, Vector3 startPos)
{
    player.position = startPos;
    player.velocity = {0.0f, 0.0f, 0.0f};
    player.moveSpeed = 5.0f;
    player.health = 100.0f;
    player.eyeHeight = 1.6f;
    player.colliderSize = {0.5f, 1.8f, 0.5f};
    player.isGrounded = false;
    player.bobTimer = 0.0f;
    player.bobOffset = 0.0f;
}

void UpdatePlayer(Player &player, Vector3 moveDirection, bool jumpPressed, float deltaTime, const std::vector<AABB> &obstacles)
{
    // Normalize horizontal input to avoid faster diagonal speed
    if (Vector3Length(moveDirection) > 0.0f)
    {
        moveDirection = Vector3Normalize(moveDirection);
    }

    // Calculate horizontal velocity
    player.velocity.x = moveDirection.x * player.moveSpeed;
    player.velocity.z = moveDirection.z * player.moveSpeed;

    // Apply jump impulse when on ground
    if (jumpPressed && player.isGrounded)
    {
        player.velocity.y = JUMP_FORCE;
        player.isGrounded = false;
    }

    // Apply gravity
    player.velocity.y += GRAVITY * deltaTime;

    // X-axis movement and obstacle collision
    player.position.x += player.velocity.x * deltaTime;
    if (CheckWorldCollision(GetPlayerAABB(player), obstacles))
    {
        player.position.x -= player.velocity.x * deltaTime;
        player.velocity.x = 0.0f;
    }

    // Z-axis movement and obstacle collision
    player.position.z += player.velocity.z * deltaTime;
    if (CheckWorldCollision(GetPlayerAABB(player), obstacles))
    {
        player.position.z -= player.velocity.z * deltaTime;
        player.velocity.z = 0.0f;
    }

    // Y-axis movement and obstacle collision
    player.position.y += player.velocity.y * deltaTime;
    player.isGrounded = false;

    if (CheckWorldCollision(GetPlayerAABB(player), obstacles))
    {
        player.position.y -= player.velocity.y * deltaTime;
        if (player.velocity.y < 0.0f)
        {
            player.isGrounded = true; // Landed on top of obstacle
        }
        player.velocity.y = 0.0f;
    }

    // Ground plane fallback (Y = 0)
    if (player.position.y <= 0.0f)
    {
        player.position.y = 0.0f;
        player.velocity.y = 0.0f;
        player.isGrounded = true;
    }

    // Head bobbing simulation during ground movement
    float speed = sqrtf(player.velocity.x * player.velocity.x + player.velocity.z * player.velocity.z);
    float targetBob = 0.0f;

    if (player.isGrounded && speed > 0.1f)
    {
        player.bobTimer += deltaTime * BOB_FREQUENCY;
        targetBob = sinf(player.bobTimer) * BOB_AMPLITUDE;
    }
    else
    {
        player.bobTimer = 0.0f;
    }

    // Smoothly interpolate bobbing offset back to rest position
    player.bobOffset += (targetBob - player.bobOffset) * 10.0f * deltaTime;
}

Vector3 GetPlayerEyePosition(const Player &player)
{
    return {player.position.x, player.position.y + player.eyeHeight + player.bobOffset, player.position.z};
}

AABB GetPlayerAABB(const Player &player)
{
    float halfW = player.colliderSize.x * 0.5f;
    float halfD = player.colliderSize.z * 0.5f;

    AABB box;
    box.min = {
        player.position.x - halfW,
        player.position.y,
        player.position.z - halfD};
    box.max = {
        player.position.x + halfW,
        player.position.y + player.colliderSize.y,
        player.position.z + halfD};
    return box;
}
