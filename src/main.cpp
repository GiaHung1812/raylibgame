#include <raylib.h>
#include <raymath.h>
#include "player.h"
#include "camera.h"

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(1280, 720, "My Game");
    DisableCursor();
    PollInputEvents();
    SetTargetFPS(100);

    // Player posistion
    Player player;
    InitPlayer(player, (Vector3){0.0f, 0.0f, 0.0f});

    // Player eye position
    GameCamera camera;
    InitGameCamera(camera, GetPlayerEyePosition(player));

    // Main game loop
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float deltaTime = dt;
        if (deltaTime > 0.05f)
            deltaTime = 0.05f;

        UpdateGameCamera(camera, GetPlayerEyePosition(player), deltaTime);

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

        if (Vector3Length(moveDir) > 0.0f)
            moveDir = Vector3Normalize(moveDir);

        UpdatePlayer(player, moveDir, deltaTime);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(10, 10);

        BeginMode3D(camera.playerCamera);
        DrawGrid(100, 1.0f);
        EndMode3D();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
