#include "enemy.h"
#include <raymath.h>
#include <algorithm>
#include <cmath>

constexpr float DETECT_RADIUS = 10.0f;
constexpr float LOSE_RADIUS = 14.0f;
constexpr float ATTACK_RANGE = 1.4f;
constexpr float ATTACK_WINDUP = 0.35f;
constexpr float ATTACK_COOLDOWN = 1.0f;
constexpr float MIN_PLAYER_SEPARATION = 0.9f;
constexpr float ENEMY_DAMAGE = 10.0f;

struct EnemyAnimClip
{
    std::vector<Rectangle> frames;
    float speed;
    bool loop;
};

static std::vector<Rectangle> MakeGridRowFrames(float cellW, float cellH, int row, int count)
{
    std::vector<Rectangle> frames;
    frames.reserve(count);
    for (int col = 0; col < count; ++col)
        frames.push_back({col * cellW, row * cellH, cellW, cellH});
    return frames;
}

static const EnemyAnimClip &GetAnimClip(EnemyType type, EnemyState state)
{
    static const EnemyAnimClip skeletonClips[4] = {
        {MakeGridRowFrames(200.0f, 200.0f, 0, 1), 0.20f, true},  // Idle
        {MakeGridRowFrames(200.0f, 200.0f, 1, 4), 0.14f, true},  // Chase
        {MakeGridRowFrames(200.0f, 200.0f, 2, 5), 0.12f, true},  // Attack
        {MakeGridRowFrames(200.0f, 200.0f, 4, 4), 0.16f, false}, // Dead
    };

    // Exact frame bounds cropped to exclude text labels in assets/deckhand.png
    static const EnemyAnimClip deckhandClips[4] = {
        {{{11, 10, 23, 37}, {45, 10, 21, 37}, {78, 10, 22, 37}}, 0.28f, true},
        {{{177, 10, 29, 37}, {210, 10, 23, 37}, {243, 10, 29, 37}, {277, 10, 28, 37}, {310, 10, 29, 37}}, 0.12f, true},
        {{{4, 105, 35, 38}, {53, 104, 25, 38}, {94, 100, 27, 43}, {135, 106, 36, 36}, {174, 105, 31, 38}}, 0.11f, true},
        {{{10, 153, 25, 38}, {51, 153, 26, 38}, {95, 153, 26, 38}, {136, 158, 26, 31}, {177, 160, 27, 29}, {217, 161, 30, 28}, {256, 169, 39, 22}, {300, 169, 40, 23}}, 0.12f, false},
    };

    size_t stateIdx = static_cast<size_t>(state);
    if (type == EnemyType::Deckhand)
        return deckhandClips[stateIdx];
    return skeletonClips[stateIdx];
}

static void ApplyStateAnimation(Enemy &enemy, EnemyState nextState)
{
    enemy.state = nextState;
    const EnemyAnimClip &clip = GetAnimClip(enemy.type, nextState);
    enemy.animFrame = 0;
    enemy.animFrameCount = static_cast<int>(clip.frames.size());
    enemy.animTimer = 0.0f;
    enemy.animSpeed = clip.speed;
    enemy.animLoop = clip.loop;
}

static void SetEnemyState(Enemy &enemy, EnemyState nextState)
{
    if (enemy.state == nextState)
        return;

    ApplyStateAnimation(enemy, nextState);
    if (nextState == EnemyState::Attack && enemy.attackCooldown < ATTACK_WINDUP)
        enemy.attackCooldown = ATTACK_WINDUP;
}

static void UpdateEnemyAnimation(Enemy &enemy, float dt)
{
    if (enemy.animFrameCount <= 1)
        return;

    enemy.animTimer += dt;
    if (enemy.animTimer < enemy.animSpeed)
        return;

    enemy.animTimer -= enemy.animSpeed;
    if (enemy.animFrame + 1 < enemy.animFrameCount)
    {
        enemy.animFrame++;
    }
    else if (enemy.animLoop)
    {
        enemy.animFrame = 0;
    }
}

EnemySprite LoadEnemySprite(const char *imagePath, float baseFrameHeight)
{
    EnemySprite sprite;
    sprite.baseFrameHeight = (baseFrameHeight > 0.0f) ? baseFrameHeight : 1.0f;
    sprite.texture = LoadTexture(imagePath);

    if (sprite.texture.id != 0)
        SetTextureFilter(sprite.texture, TEXTURE_FILTER_POINT);

    return sprite;
}

void UnloadEnemySprite(EnemySprite &sprite)
{
    if (sprite.texture.id != 0)
    {
        UnloadTexture(sprite.texture);
        sprite.texture = {};
    }
}

void InitEnemy(Enemy &enemy, Vector3 startPos, EnemyType type)
{
    enemy.type = type;
    enemy.position = startPos;
    enemy.velocity = {0.0f, 0.0f, 0.0f};
    enemy.radius = 0.4f;
    enemy.height = 1.75f;
    enemy.speed = (type == EnemyType::Deckhand) ? 3.1f : 2.6f;
    enemy.maxHealth = (type == EnemyType::Deckhand) ? 120.0f : 100.0f;
    enemy.health = enemy.maxHealth;

    ApplyStateAnimation(enemy, EnemyState::Idle);

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
        SetEnemyState(enemy, EnemyState::Dead);
        enemy.deathTimer = 1.2f;
    }
    else if (enemy.state == EnemyState::Idle)
    {
        SetEnemyState(enemy, EnemyState::Chase);
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

static void TranslateEnemyWithCollision(Enemy &enemy, Vector3 delta, const CollisonList &obstacles)
{
    enemy.position.x += delta.x;
    if (CheckWorldCollision(GetEnemyAABB(enemy), obstacles))
        enemy.position.x -= delta.x;

    enemy.position.z += delta.z;
    if (CheckWorldCollision(GetEnemyAABB(enemy), obstacles))
        enemy.position.z -= delta.z;
}

static void MoveEnemyWithCollision(Enemy &enemy, Vector3 moveVel, float dt, const CollisonList &obstacles)
{
    TranslateEnemyWithCollision(enemy, Vector3Scale(moveVel, dt), obstacles);
}

void UpdateEnemies(std::vector<Enemy> &enemies, Vector3 playerPos, float &playerHealth, float dt, const CollisonList &obstacles)
{
    for (auto &enemy : enemies)
    {
        if (enemy.hitFlashTimer > 0.0f)
            enemy.hitFlashTimer -= dt;
        if (enemy.attackCooldown > 0.0f)
            enemy.attackCooldown -= dt;

        UpdateEnemyAnimation(enemy, dt);

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

        // Keep minimum spacing from the player camera without pushing into walls
        if (dist < MIN_PLAYER_SEPARATION)
        {
            Vector3 awayDir = (dist > 0.001f) ? Vector3Scale(toPlayer, -1.0f / dist) : Vector3{0.0f, 0.0f, 1.0f};
            TranslateEnemyWithCollision(enemy, Vector3Scale(awayDir, MIN_PLAYER_SEPARATION - dist), obstacles);
            toPlayer = {playerPos.x - enemy.position.x, 0.0f, playerPos.z - enemy.position.z};
            dist = Vector3Length(toPlayer);
        }

        switch (enemy.state)
        {
        case EnemyState::Idle:
            if (dist <= DETECT_RADIUS)
                SetEnemyState(enemy, EnemyState::Chase);
            break;

        case EnemyState::Chase:
        {
            if (dist > LOSE_RADIUS)
            {
                SetEnemyState(enemy, EnemyState::Idle);
                break;
            }
            if (dist <= ATTACK_RANGE)
            {
                SetEnemyState(enemy, EnemyState::Attack);
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
                SetEnemyState(enemy, EnemyState::Chase);
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

void DrawEnemies(const std::vector<Enemy> &enemies, const Camera3D &camera, const std::vector<EnemySprite> &sprites)
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
        size_t typeIdx = static_cast<size_t>(enemy.type);

        if (typeIdx >= sprites.size() || sprites[typeIdx].texture.id == 0)
        {
            Vector3 fallbackPos = {enemy.position.x, enemy.position.y + enemy.height * 0.5f, enemy.position.z};
            DrawCube(fallbackPos, enemy.radius * 2.0f, enemy.height, enemy.radius * 2.0f, MAGENTA);
            continue;
        }

        const EnemySprite &sprite = sprites[typeIdx];
        const EnemyAnimClip &clip = GetAnimClip(enemy.type, enemy.state);
        int frameIdx = Clamp(enemy.animFrame, 0, static_cast<int>(clip.frames.size()) - 1);
        Rectangle srcRect = clip.frames[frameIdx];

        // Preserve sprite aspect ratio relative to standing frame height
        float worldHeight = enemy.height * (srcRect.height / sprite.baseFrameHeight);
        float worldWidth = worldHeight * (srcRect.width / srcRect.height);
        Vector3 drawPos = {enemy.position.x, enemy.position.y + worldHeight * 0.5f, enemy.position.z};

        Color tint = WHITE;
        if (enemy.state == EnemyState::Dead)
            tint = Fade(WHITE, Clamp(enemy.deathTimer / 0.6f, 0.0f, 1.0f));
        else if (enemy.hitFlashTimer > 0.0f)
            tint = RED;

        DrawBillboardRec(camera, sprite.texture, srcRect, drawPos, {worldWidth, worldHeight}, tint);
    }
}
