#include "enemy.h"
#include <raymath.h>
#include <algorithm>
#include <cmath>

constexpr float DETECT_RADIUS = 10.0f;
constexpr float LOSE_RADIUS = 14.0f;
constexpr float ATTACK_RANGE = 1.4f;
constexpr float ATTACK_COOLDOWN = 1.0f;
constexpr float ENEMY_DAMAGE = 10.0f;

Texture2D LoadOrCreateEnemyTexture(const char *imagePath)
{
    if (FileExists(imagePath))
        return LoadTexture(imagePath);

    // Procedural pixel-art skeleton placeholder when no sprite file exists
    const char *pattern[16] = {
        "................",
        ".....XXXXXX.....",
        "....XXXXXXXX....",
        "....XX.XX.XX....",
        "....XXXXXXXX....",
        ".....XX..XX.....",
        "......XXXX......",
        "....XXXXXXXX....",
        "...XX.XXXX.XX...",
        "...XX.XXXX.XX...",
        "......XXXX......",
        ".....XX..XX.....",
        ".....XX..XX.....",
        ".....XX..XX.....",
        "....XXX..XXX....",
        "................"};

    Image img = GenImageColor(16, 16, BLANK);
    for (int y = 0; y < 16; ++y)
    {
        for (int x = 0; x < 16; ++x)
        {
            if (pattern[y][x] == 'X')
                ImageDrawPixel(&img, x, y, LIGHTGRAY);
        }
    }

    // Highlight glowing red eye sockets
    ImageDrawPixel(&img, 6, 3, RED);
    ImageDrawPixel(&img, 9, 3, RED);

    Texture2D tex = LoadTextureFromImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    UnloadImage(img);
    return tex;
}

void InitEnemy(Enemy &enemy, Vector3 startPos)
{
    enemy.position = startPos;
    enemy.velocity = {0.0f, 0.0f, 0.0f};
    enemy.radius = 0.4f;
    enemy.height = 1.6f;
    enemy.speed = 2.6f;
    enemy.maxHealth = 100.0f;
    enemy.health = enemy.maxHealth;
    enemy.state = EnemyState::Idle;
    enemy.lastAttackId = -1;
    enemy.hitFlashTimer = 0.0f;
    enemy.attackCooldown = 0.0f;
    enemy.deathTimer = 0.0f;
}

AABB GetEnemyAABB(const Enemy &enemy)
{
    return {
        {enemy.position.x - enemy.radius, enemy.position.y, enemy.position.z - enemy.radius},
        {enemy.position.x + enemy.radius, enemy.position.y + enemy.height, enemy.position.z + enemy.radius}};
}

bool DamageEnemy(Enemy &enemy, float damage, int attackId, Vector3 knockbackDir, float knockbackForce)
{
    // Ignore already dead enemies and prevent multiple hits from the same swing
    if (enemy.state == EnemyState::Dead || enemy.lastAttackId == attackId)
        return false;

    enemy.lastAttackId = attackId;
    enemy.health -= damage;
    enemy.hitFlashTimer = 0.15f;

    // Apply horizontal knockback impulse away from attacker
    Vector3 flatDir = {knockbackDir.x, 0.0f, knockbackDir.z};
    if (Vector3LengthSqr(flatDir) > 0.0001f)
    {
        flatDir = Vector3Normalize(flatDir);
        enemy.velocity = Vector3Scale(flatDir, knockbackForce);
    }

    if (enemy.health <= 0.0f)
    {
        enemy.health = 0.0f;
        enemy.state = EnemyState::Dead;
        enemy.deathTimer = 1.0f;
    }
    else if (enemy.state == EnemyState::Idle)
    {
        enemy.state = EnemyState::Chase;
    }

    return true;
}

static Vector3 ComputeSeparation(const Enemy &self, const std::vector<Enemy> &enemies)
{
    Vector3 push = {0.0f, 0.0f, 0.0f};
    const float minSep = self.radius * 2.2f;

    for (const auto &other : enemies)
    {
        if (&self == &other || other.state == EnemyState::Dead)
            continue;

        Vector3 diff = {self.position.x - other.position.x, 0.0f, self.position.z - other.position.z};
        float distSq = diff.x * diff.x + diff.z * diff.z;
        if (distSq > 0.0001f && distSq < minSep * minSep)
        {
            float dist = sqrtf(distSq);
            float strength = (minSep - dist) / minSep;
            push.x += (diff.x / dist) * strength;
            push.z += (diff.z / dist) * strength;
        }
    }
    return push;
}

static void MoveEnemyWithCollision(Enemy &enemy, Vector3 moveVel, float dt, const CollisonList &obstacles)
{
    enemy.position.x += moveVel.x * dt;
    if (CheckWorldCollision(GetEnemyAABB(enemy), obstacles))
        enemy.position.x -= moveVel.x * dt;

    enemy.position.z += moveVel.z * dt;
    if (CheckWorldCollision(GetEnemyAABB(enemy), obstacles))
        enemy.position.z -= moveVel.z * dt;
}

void UpdateEnemies(std::vector<Enemy> &enemies, Vector3 playerPos, float &playerHealth, float dt, const CollisonList &obstacles)
{
    for (auto &enemy : enemies)
    {
        if (enemy.hitFlashTimer > 0.0f)
            enemy.hitFlashTimer -= dt;
        if (enemy.attackCooldown > 0.0f)
            enemy.attackCooldown -= dt;

        // Apply and damp knockback velocity with wall collision
        if (Vector3LengthSqr(enemy.velocity) > 0.001f)
        {
            MoveEnemyWithCollision(enemy, enemy.velocity, dt, obstacles);
            enemy.velocity = Vector3Lerp(enemy.velocity, {0.0f, 0.0f, 0.0f}, Clamp(dt * 10.0f, 0.0f, 1.0f));
        }
        else
        {
            enemy.velocity = {0.0f, 0.0f, 0.0f};
        }

        if (enemy.state == EnemyState::Dead)
        {
            enemy.deathTimer -= dt;
            continue;
        }

        // Brief stagger pause while reeling from hit flash
        if (enemy.hitFlashTimer > 0.0f)
            continue;

        Vector3 toPlayer = {playerPos.x - enemy.position.x, 0.0f, playerPos.z - enemy.position.z};
        float dist = Vector3Length(toPlayer);

        switch (enemy.state)
        {
        case EnemyState::Idle:
            if (dist <= DETECT_RADIUS)
                enemy.state = EnemyState::Chase;
            break;

        case EnemyState::Chase:
        {
            if (dist > LOSE_RADIUS)
            {
                enemy.state = EnemyState::Idle;
                break;
            }
            if (dist <= ATTACK_RANGE)
            {
                enemy.state = EnemyState::Attack;
                break;
            }

            Vector3 dir = (dist > 0.001f) ? Vector3Scale(toPlayer, 1.0f / dist) : Vector3{0.0f, 0.0f, 0.0f};
            Vector3 sep = ComputeSeparation(enemy, enemies);
            Vector3 moveDir = Vector3Add(dir, Vector3Scale(sep, 0.8f));
            if (Vector3LengthSqr(moveDir) > 0.001f)
                moveDir = Vector3Normalize(moveDir);

            MoveEnemyWithCollision(enemy, Vector3Scale(moveDir, enemy.speed), dt, obstacles);
            break;
        }

        case EnemyState::Attack:
            if (dist > ATTACK_RANGE + 0.4f)
            {
                enemy.state = EnemyState::Chase;
                break;
            }
            if (enemy.attackCooldown <= 0.0f)
            {
                playerHealth = fmaxf(0.0f, playerHealth - ENEMY_DAMAGE);
                enemy.attackCooldown = ATTACK_COOLDOWN;
            }
            break;

        case EnemyState::Dead:
            break;
        }
    }

    // Remove enemies whose death timer has expired
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                       [](const Enemy &e) { return e.state == EnemyState::Dead && e.deathTimer <= 0.0f; }),
        enemies.end());
}

void DrawEnemies(const std::vector<Enemy> &enemies, const Camera3D &camera, Texture2D sprite)
{
    // Sort back-to-front so transparent billboards overlap properly
    std::vector<size_t> order(enemies.size());
    for (size_t i = 0; i < enemies.size(); ++i)
        order[i] = i;

    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return Vector3DistanceSqr(enemies[a].position, camera.position) >
               Vector3DistanceSqr(enemies[b].position, camera.position);
    });

    for (size_t idx : order)
    {
        const Enemy &enemy = enemies[idx];
        Vector3 drawPos = {enemy.position.x, enemy.position.y + enemy.height * 0.5f, enemy.position.z};

        Color tint = WHITE;
        if (enemy.state == EnemyState::Dead)
            tint = Fade(DARKGRAY, Clamp(enemy.deathTimer, 0.0f, 1.0f));
        else if (enemy.hitFlashTimer > 0.0f)
            tint = RED;

        DrawBillboard(camera, sprite, drawPos, enemy.height, tint);
    }
}
