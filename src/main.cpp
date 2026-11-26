#include <raylib.h>
#include <raymath.h>
#include "player.h"
#include "camera.h"
#include "collisions.h"
#include "world.h"
#include "weapon.h"

enum class GameState
{
    Title,
    Playing,
    GameOver,
    Victory
};

static void ResetLevel(World &world, Player &player, GameCamera &camera, Weapon &weapon)
{
    Vector3 playerSpawn = {0.0f, 0.0f, 0.0f};
    if (!LoadDungeonFromImage(world, "assets/dungeon_map.png", playerSpawn))
        InitWorld(world);

    // Spawn default test enemies if map has no red enemy pixels
    if (world.enemies.empty())
    {
        Vector3 offsets[2] = {{4.0f, 0.0f, -6.0f}, {-4.0f, 0.0f, -6.0f}};
        for (const auto &off : offsets)
        {
            Enemy enemy;
            InitEnemy(enemy, Vector3Add(playerSpawn, off));
            if (!CheckWorldCollision(GetEnemyAABB(enemy), world.obstacles))
                world.enemies.push_back(enemy);
        }
    }

    InitPlayer(player, playerSpawn);
    InitGameCamera(camera, GetPlayerEyePosition(player));
    ResetWeapon(weapon);
}

static void EnterPlayingState(World &world, Player &player, GameCamera &camera, Weapon &weapon,
                              float &hitIndicatorTimer, float &hitStopTimer, GameState &currentState)
{
    ResetLevel(world, player, camera, weapon);
    hitIndicatorTimer = 0.0f;
    hitStopTimer = 0.0f;
    DisableCursor();
    PollInputEvents();
    currentState = GameState::Playing;
}

static void UpdatePlaying(World &world, Player &player, GameCamera &camera, Weapon &weapon,
                          Vector3 &lastHitPoint, float &hitIndicatorTimer, float &hitStopTimer, float dt)
{
    UpdateGameCamera(camera, GetPlayerEyePosition(player));

    if (hitStopTimer > 0.0f)
    {
        hitStopTimer -= dt;
        return;
    }

    Vector3 moveDir = {0.0f, 0.0f, 0.0f};
    Vector3 forward = GetCameraForward(camera);
    Vector3 right = GetCameraRight(camera);

    if (IsKeyDown(KEY_W))
        moveDir = Vector3Add(moveDir, forward);
    if (IsKeyDown(KEY_S))
        moveDir = Vector3Subtract(moveDir, forward);
    if (IsKeyDown(KEY_D))
        moveDir = Vector3Add(moveDir, right);
    if (IsKeyDown(KEY_A))
        moveDir = Vector3Subtract(moveDir, right);

    bool jumpPressed = IsKeyPressed(KEY_SPACE);
    UpdatePlayer(player, moveDir, jumpPressed, dt, world.obstacles);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        StartWeaponSwing(weapon);

    bool isMoving = player.isGrounded && (player.velocity.x * player.velocity.x + player.velocity.z * player.velocity.z > 0.01f);
    if (UpdateWeapon(weapon, isMoving, dt))
    {
        Vector3 rayDir = Vector3Normalize(Vector3Subtract(camera.playerCamera.target, camera.playerCamera.position));
        Ray attackRay = {camera.playerCamera.position, rayDir};
        const float meleeRange = 2.5f;

        // Clamp melee reach against closest wall so swings cannot hit through walls
        RayCollision wallHit = RaycastWorld(attackRay, meleeRange, world.obstacles);
        float maxHitDist = wallHit.hit ? wallHit.distance : meleeRange;

        Enemy *closestEnemy = nullptr;
        RayCollision closestEnemyHit = {0};

        for (auto &enemy : world.enemies)
        {
            if (enemy.state == EnemyState::Dead)
                continue;

            AABB box = GetEnemyAABB(enemy);
            RayCollision hit = GetRayCollisionBox(attackRay, {box.min, box.max});
            if (hit.hit && hit.distance >= 0.0f && hit.distance <= maxHitDist &&
                (!closestEnemy || hit.distance < closestEnemyHit.distance))
            {
                closestEnemy = &enemy;
                closestEnemyHit = hit;
            }
        }

        float knockbackForce = (weapon.currentAttack == SwordAttackType::Stab) ? 10.0f : 6.0f;
        if (closestEnemy && DamageEnemy(*closestEnemy, GetWeaponDamage(weapon), weapon.attackId, rayDir, knockbackForce))
        {
            lastHitPoint = closestEnemyHit.point;
            hitIndicatorTimer = 0.4f;
            hitStopTimer = (weapon.currentAttack == SwordAttackType::Stab) ? 0.055f : 0.035f;
        }
        else if (wallHit.hit)
        {
            lastHitPoint = wallHit.point;
            hitIndicatorTimer = 0.4f;
        }
    }

    UpdateEnemies(world.enemies, player.position, player.health, dt, world.obstacles);
}

static void DrawCenteredText(const char *text, int y, int fontSize, Color color)
{
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, (GetScreenWidth() - textWidth) / 2, y, fontSize, color);
}

static void DrawHUD(const Player &player, const World &world, bool hitActive)
{
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // Screen center crosshair
    int midX = screenW / 2;
    int midY = screenH / 2;
    Color crosshairColor = hitActive ? ORANGE : WHITE;
    DrawCircle(midX, midY, 2.5f, Fade(BLACK, 0.5f));
    DrawCircle(midX, midY, 1.5f, crosshairColor);

    // Health bar (bottom-left)
    const int barX = 20;
    const int barY = screenH - 46;
    const int barW = 220;
    const int barH = 24;
    float hpRatio = Clamp(player.health / 100.0f, 0.0f, 1.0f);

    DrawRectangle(barX, barY, barW, barH, Fade(BLACK, 0.65f));
    DrawRectangle(barX + 2, barY + 2, (int)((barW - 4) * hpRatio), barH - 4, RED);
    DrawRectangleLines(barX, barY, barW, barH, DARKGRAY);
    DrawText(TextFormat("HP: %.0f / 100", player.health), barX + 10, barY + 3, 18, WHITE);

    // Remaining enemies counter
    DrawText(TextFormat("Enemies: %d", (int)world.enemies.size()), 10, 35, 20, DARKGRAY);
}

static void DrawStateOverlay(GameState state)
{
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.6f));

    int centerY = screenH / 2;
    if (state == GameState::Title)
    {
        DrawCenteredText("DUNGEON BLADE", centerY - 60, 48, WHITE);
        DrawCenteredText("WASD: Move | Space: Jump | Left Click: Sword Combo", centerY + 5, 20, LIGHTGRAY);
        DrawCenteredText("PRESS ENTER TO START", centerY + 45, 24, GOLD);
    }
    else if (state == GameState::GameOver)
    {
        DrawCenteredText("GAME OVER", centerY - 50, 48, RED);
        DrawCenteredText("PRESS R OR ENTER TO RETRY", centerY + 20, 24, WHITE);
    }
    else if (state == GameState::Victory)
    {
        DrawCenteredText("VICTORY!", centerY - 50, 48, GOLD);
        DrawCenteredText("All enemies defeated!", centerY + 5, 20, LIGHTGRAY);
        DrawCenteredText("PRESS R OR ENTER TO PLAY AGAIN", centerY + 40, 24, WHITE);
    }
}

int main()
{
    // Window initialization
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "My Game");
    EnableCursor();
    SetTargetFPS(100);

    World world;
    Player player;
    GameCamera camera;
    Weapon weapon;
    InitWeapon(weapon);

    // Indexed by EnemyType enum order
    std::vector<EnemySprite> enemySprites = {
        LoadEnemySprite("assets/skeletonSheet.png", 5, 5)};
    ResetLevel(world, player, camera, weapon);

    GameState currentState = GameState::Title;
    Vector3 lastHitPoint = {0.0f, 0.0f, 0.0f};
    float hitIndicatorTimer = 0.0f;
    float hitStopTimer = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.05f)
            dt = 0.05f;

        switch (currentState)
        {
        case GameState::Title:
            if (IsKeyPressed(KEY_ENTER))
                EnterPlayingState(world, player, camera, weapon, hitIndicatorTimer, hitStopTimer, currentState);
            break;

        case GameState::Playing:
            UpdatePlaying(world, player, camera, weapon, lastHitPoint, hitIndicatorTimer, hitStopTimer, dt);
            if (player.health <= 0.0f)
            {
                currentState = GameState::GameOver;
                EnableCursor();
            }
            else if (world.enemies.empty())
            {
                currentState = GameState::Victory;
                EnableCursor();
            }
            break;

        case GameState::GameOver:
        case GameState::Victory:
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER))
                EnterPlayingState(world, player, camera, weapon, hitIndicatorTimer, hitStopTimer, currentState);
            break;
        }

        if (hitIndicatorTimer > 0.0f)
            hitIndicatorTimer -= dt;

        // Render pass
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera.playerCamera);
        DrawGrid(100, 1.0f);
        DrawWorld(world);
        DrawEnemies(world.enemies, camera.playerCamera, enemySprites);
        if (hitIndicatorTimer > 0.0f)
            DrawSphere(lastHitPoint, 0.08f, ORANGE);
        DrawWeapon(weapon, camera.playerCamera);
        EndMode3D();

        DrawFPS(10, 10);
        if (currentState == GameState::Playing)
            DrawHUD(player, world, hitIndicatorTimer > 0.0f);
        else
            DrawStateOverlay(currentState);

        EndDrawing();
    }

    for (auto &sprite : enemySprites)
        UnloadEnemySprite(sprite);
    UnloadWeapon(weapon);
    CloseWindow();
    return 0;
}
