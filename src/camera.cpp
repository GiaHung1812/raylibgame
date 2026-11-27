#include "camera.h"
#include <raymath.h>

void InitGameCamera(GameCamera &cam, Vector3 startPosition)
{
    cam.playerCamera = {};
    cam.playerCamera.position = startPosition;
    cam.playerCamera.target = Vector3Add(startPosition, (Vector3){0.0f, 0.0f, -1.0f});
    cam.playerCamera.up = (Vector3){0.0f, 1.0f, 0.0f};
    cam.playerCamera.fovy = 60.0f;
    cam.playerCamera.projection = CAMERA_PERSPECTIVE;

    cam.mouseSensitivity = 0.0025f;
    cam.pitch = 0.0f;
    cam.yaw = 0.0f;
}

void UpdateGameCamera(GameCamera &cam, Vector3 targetEyePos)
{
    Vector2 mouseDelta = GetMouseDelta();

    cam.yaw += mouseDelta.x * cam.mouseSensitivity;
    cam.pitch -= mouseDelta.y * cam.mouseSensitivity;

    const float maxPitch = 89.0f * DEG2RAD;
    if (cam.pitch > maxPitch)
        cam.pitch = maxPitch;
    if (cam.pitch < -maxPitch)
        cam.pitch = -maxPitch;

    Vector3 forward = {
        sinf(cam.yaw) * cosf(cam.pitch),
        sinf(cam.pitch),
        -cosf(cam.yaw) * cosf(cam.pitch)};

    cam.playerCamera.position = targetEyePos;
    cam.playerCamera.target = Vector3Add(targetEyePos, forward);
}

Vector3 GetCameraForward(const GameCamera &cam)
{
    Vector3 fwd = {sinf(cam.yaw), 0.0f, -cosf(cam.yaw)};
    return Vector3Normalize(fwd);
}

Vector3 GetCameraRight(const GameCamera &cam)
{
    Vector3 right = {cosf(cam.yaw), 0.0f, sinf(cam.yaw)};
    return Vector3Normalize(right);
}
