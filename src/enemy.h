#pragma once

#include <raylib.h>
#include <vector>
#include "collisions.h"

enum class EnemyState
{
    Idle,
    Chase,
    Attack,
    Dead
};

struct Enemy
{
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 velocity = {0.0f, 0.0f, 0.0f};
    float radius = 0.4f;
    float height = 1.6f;
    float speed = 2.6f;

    float health = 100.0f;
    float maxHealth = 100.0f;
    EnemyState state = EnemyState::Idle;

    int lastAttackId = -1;
    float hitFlashTimer = 0.0f;
    float attackCooldown = 0.0f;
    float deathTimer = 0.0f;
};

Texture2D LoadOrCreateEnemyTexture(const char *imagePath);
void InitEnemy(Enemy &enemy, Vector3 startPos);
AABB GetEnemyAABB(const Enemy &enemy);
bool DamageEnemy(Enemy &enemy, float damage, int attackId, Vector3 knockbackDir, float knockbackForce);
void UpdateEnemies(std::vector<Enemy> &enemies, Vector3 playerPos, float &playerHealth, float dt, const CollisonList &obstacles);
void DrawEnemies(const std::vector<Enemy> &enemies, const Camera3D &camera, Texture2D sprite);
