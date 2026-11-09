#include <iostream>
#include <raylib.h>
#include <raymath.h>

using namespace std;

int main()
{
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Raylib Game");
    SetTargetFPS(120);

    Camera3D camera = {
        position : {10.0f, 1.8f, 4.0f},
        target : {0.0f, 0.0f, 0.0f},
        up : {0.0f, 1.0f, 0.0f},
        fovy : 60.0f,
        projection : CAMERA_PERSPECTIVE
    };

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        DisableCursor();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawCube({0.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 1.0f, RED);
        EndMode3D();

        EndDrawing();
        DrawFPS(10, 10);
    }
    CloseWindow();
    return 0;
}