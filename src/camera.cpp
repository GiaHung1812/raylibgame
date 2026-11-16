#include "camera.h"
#include <raymath.h>

void InitGameCamera(GameCamera &cam, Vector3 startPosition)
{
    cam.playerCamera = {0};
    cam.playerCamera.position = startPosition;
    cam.playerCamera.target = Vector3Add(startPosition, (Vector3){0.0f, 0.0f, -1.0f});
    cam.playerCamera.up = (Vector3){0.0f, 1.0f, 0.0f};
    cam.playerCamera.fovy = 60.0f;
    cam.playerCamera.projection = CAMERA_PERSPECTIVE;

    cam.mouseSensitivity = 0.0025f;
    cam.pitch = 0.0f;
    cam.yaw = 0.0f;
}

void UpdateGameCamera(GameCamera &cam, Vector3 targetEyePos, float deltaTime)
{
    (void)deltaTime;
    Vector2 mouseDelta = GetMouseDelta();

    // Update camera view rotation with mouse delta
    cam.yaw += mouseDelta.x * cam.mouseSensitivity;
    cam.pitch -= mouseDelta.y * cam.mouseSensitivity;

    // Clamp pitch angle to avoid camera flipping
    const float maxPitch = 89.0f * DEG2RAD;
    if (cam.pitch > maxPitch)
        cam.pitch = maxPitch;
    if (cam.pitch < -maxPitch)
        cam.pitch = -maxPitch;

    // Calculate forward view vector from yaw and pitch
    Vector3 forward = {
        sinf(cam.yaw) * cosf(cam.pitch),
        sinf(cam.pitch),
        -cosf(cam.yaw) * cosf(cam.pitch)};

    // First-person camera placed at player's eye level
    cam.playerCamera.position = targetEyePos;
    cam.playerCamera.target = Vector3Add(targetEyePos, forward);
}

Vector3 GetCameraForward(const GameCamera &cam)
{
    // Forward direction on horizontal XZ plane (ignore Y for ground movement)
    Vector3 fwd = {sinf(cam.yaw), 0.0f, -cosf(cam.yaw)};
    return Vector3Normalize(fwd);
}

Vector3 GetCameraRight(const GameCamera &cam)
{
    // Perpendicular right vector on horizontal XZ plane
    Vector3 right = {cosf(cam.yaw), 0.0f, sinf(cam.yaw)};
    return Vector3Normalize(right);
}
