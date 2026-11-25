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

    Vector3 lastHitPoint = {0.0f, 0.0f, 0.0f};
    float hitIndicatorTimer = 0.0f;

    // Main game loop
    while (!WindowShouldClose())
    {
        // Delta time clamped to prevent tunneling during lag spikes
        float dt = GetFrameTime();
        if (dt > 0.05f)
            dt = 0.05f;

        // Camera orientation update
        UpdateGameCamera(camera, GetPlayerEyePosition(player));

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
            RayCollision hit = RaycastWorld(attackRay, 2.5f, world.obstacles);
            if (hit.hit)
            {
                lastHitPoint = hit.point;
                hitIndicatorTimer = 0.4f;
            }
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
        if (hitIndicatorTimer > 0.0f)
            DrawSphere(lastHitPoint, 0.08f, ORANGE);
        DrawWeapon(weapon, camera.playerCamera);
        EndMode3D();

        // Screen center crosshair
        int midX = GetScreenWidth() / 2;
        int midY = GetScreenHeight() / 2;
        Color crosshairColor = (hitIndicatorTimer > 0.0f) ? ORANGE : WHITE;
        DrawCircle(midX, midY, 2.5f, Fade(BLACK, 0.5f));
        DrawCircle(midX, midY, 1.5f, crosshairColor);

        EndDrawing();
    }

    UnloadWeapon(weapon);
    CloseWindow();
    return 0;
}
