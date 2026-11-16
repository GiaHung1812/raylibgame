#pragma once
#include <raylib.h>

struct GameCamera
{
    Camera3D playerCamera;
    float mouseSensitivity;
    float pitch; // Vertical look angle (radians)
    float yaw;   // Horizontal look angle (radians)
};

void InitGameCamera(GameCamera &cam, Vector3 startPosition);
void UpdateGameCamera(GameCamera &cam, Vector3 targetEyePos, float deltaTime);
Vector3 GetCameraForward(const GameCamera &cam);
Vector3 GetCameraRight(const GameCamera &cam);
