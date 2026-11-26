#include <raylib.h>
#include <raymath.h>
#include "player.h"
#include "camera.h"
#include "collisions.h"
#include "world.h"
#include "weapon.h"

int main()
{
    // Window initialization
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "My Game");
    DisableCursor();
    PollInputEvents();
    SetTargetFPS(100);

    // World setup and dungeon generation from image
    World world;
    Vector3 playerSpawn = {0.0f, 0.0f, 0.0f};
    if (!LoadDungeonFromImage(world, "assets/dungeon_map.png", playerSpawn))
        InitWorld(world);

    Player player;
    InitPlayer(player, playerSpawn);

    GameCamera camera;
    InitGameCamera(camera, GetPlayerEyePosition(player));

    Weapon weapon;
    InitWeapon(weapon);

    Texture2D enemyTexture = LoadOrCreateEnemyTexture("assets/enemy.png");

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

    Vector3 lastHitPoint = {0.0f, 0.0f, 0.0f};
    float hitIndicatorTimer = 0.0f;
    float hitStopTimer = 0.0f;

    // Main game loop
    while (!WindowShouldClose())
    {
        // Delta time clamped to prevent tunneling during lag spikes
        float dt = GetFrameTime();
        if (dt > 0.05f)
            dt = 0.05f;

        // Camera orientation update
        UpdateGameCamera(camera, GetPlayerEyePosition(player));

        // Brief freeze-frame on melee impact for weightier combat feel
        if (hitStopTimer > 0.0f)
        {
            hitStopTimer -= dt;
        }
        else
        {
            // Movement input relative to camera orientation
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

            // Jump input
            bool jumpPressed = IsKeyPressed(KEY_SPACE);

            // Player physics, gravity, and obstacle collision
            UpdatePlayer(player, moveDir, jumpPressed, dt, world.obstacles);

            // Weapon input, bobbing state, and swing update
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

        if (hitIndicatorTimer > 0.0f)
            hitIndicatorTimer -= dt;

        // Render pass
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(10, 10);

        BeginMode3D(camera.playerCamera);
        DrawGrid(100, 1.0f);
        DrawWorld(world);
        DrawEnemies(world.enemies, camera.playerCamera, enemyTexture);
        if (hitIndicatorTimer > 0.0f)
            DrawSphere(lastHitPoint, 0.08f, ORANGE);
        DrawWeapon(weapon, camera.playerCamera);
        EndMode3D();

        // Screen center crosshair and minimal status readout
        int midX = GetScreenWidth() / 2;
        int midY = GetScreenHeight() / 2;
        Color crosshairColor = (hitIndicatorTimer > 0.0f) ? ORANGE : WHITE;
        DrawCircle(midX, midY, 2.5f, Fade(BLACK, 0.5f));
        DrawCircle(midX, midY, 1.5f, crosshairColor);
        DrawText(TextFormat("HP: %.0f | Enemies: %d", player.health, (int)world.enemies.size()), 10, 35, 20, DARKGRAY);

        EndDrawing();
    }

    UnloadTexture(enemyTexture);
    UnloadWeapon(weapon);
    CloseWindow();
    return 0;
}
