#pragma once

#include <raylib.h>
#include <vector>
#include "collisions.h"

enum class EnemyType
{
    Skeleton
};

enum class EnemyState
{
    Idle,
    Chase,
    Attack,
    Dead
};

struct EnemySprite
{
    Texture2D texture = {0};
    int cols = 1;
    int rows = 1;
};

struct Enemy
{
    EnemyType type = EnemyType::Skeleton;
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 velocity = {0.0f, 0.0f, 0.0f};
    float radius = 0.4f;
    float height = 1.75f;
    float speed = 2.6f;

    float health = 100.0f;
    float maxHealth = 100.0f;
    EnemyState state = EnemyState::Idle;

    int animRow = 0;
    int animFrame = 0;
    int animFrameCount = 1;
    float animTimer = 0.0f;
    float animSpeed = 0.15f;
    bool animLoop = true;

    int lastAttackId = -1;
    float hitFlashTimer = 0.0f;
    float attackCooldown = 0.0f;
    float deathTimer = 0.0f;
};

EnemySprite LoadEnemySprite(const char *imagePath, int cols = 1, int rows = 1);
void UnloadEnemySprite(EnemySprite &sprite);
void InitEnemy(Enemy &enemy, Vector3 startPos, EnemyType type = EnemyType::Skeleton);
AABB GetEnemyAABB(const Enemy &enemy);
bool DamageEnemy(Enemy &enemy, float damage, int attackId, Vector3 knockbackDir, float knockbackForce);
void UpdateEnemies(std::vector<Enemy> &enemies, Vector3 playerPos, float &playerHealth, float dt, const CollisonList &obstacles);
void DrawEnemies(const std::vector<Enemy> &enemies, const Camera3D &camera, const std::vector<EnemySprite> &sprites);
