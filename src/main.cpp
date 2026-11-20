#include <raylib.h>
#include <raymath.h>
#include "player.h"
#include "camera.h"
#include "collisions.h"
#include "world.h"

int main()
{
    // Window initialization
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "My Game");
    DisableCursor();
    PollInputEvents();
    SetTargetFPS(100);

    // Entity and world setup
    Player player;
    InitPlayer(player, (Vector3){0.0f, 0.0f, 0.0f});

    GameCamera camera;
    InitGameCamera(camera, GetPlayerEyePosition(player));

    World world;
    InitWorld(world);

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

        // Render pass
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(10, 10);

        BeginMode3D(camera.playerCamera);
        DrawGrid(100, 1.0f);
        DrawWorld(world);
        EndMode3D();

        // Screen center crosshair
        int midX = GetScreenWidth() / 2;
        int midY = GetScreenHeight() / 2;
        DrawCircle(midX, midY, 2.5f, Fade(BLACK, 0.5f));
        DrawCircle(midX, midY, 1.5f, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
