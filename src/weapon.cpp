#include "weapon.h"
#include <raymath.h>

void InitWeapon(Weapon &weapon)
{
    weapon.state = WeaponState::Idle;
    weapon.forwardOffset = 0.55f;
    weapon.sideOffset = 0.28f;
    weapon.verticalOffset = -0.30f;
    weapon.baseRollDeg = -15.0f;
    weapon.scale = {0.8f, 0.8f, 0.8f};

    weapon.model = LoadModel("assets/sword.obj");
}

static void UpdateRightSlashMotion(Weapon &weapon, float t)
{
    // Fast downstroke followed by deceleration
    float speedFactor = 2.0f;
    float tFast = Clamp(t * speedFactor, 0.0f, 1.0f);
    float arc = sinf(tFast * (PI * 0.6f));

    float swingAmount = 0.15f;
    float verticalSwingAmount = 0.25f;
    float horizontalSwingAmount = 0.25f;

    weapon.swingOffset = arc * swingAmount;
    weapon.verticalSwingOffset = -arc * verticalSwingAmount;
    weapon.horizontalSwingOffset = -arc * horizontalSwingAmount;

    weapon.attackYawDeg = arc * -35.0f;
    weapon.attackRollDeg = arc * 25.0f;
}

void UpdateWeapon(Weapon &weapon, float dt)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && weapon.state == WeaponState::Idle)
    {
        weapon.state = WeaponState::Attacking;
        weapon.swingTimer = 0.0f;
    }

    if (weapon.state == WeaponState::Attacking)
    {
        weapon.swingTimer += dt;
        float t = weapon.swingTimer / weapon.swingDuration;
        t = Clamp(t, 0.0f, 1.0f);

        UpdateRightSlashMotion(weapon, t);

        if (weapon.swingTimer >= weapon.swingDuration)
        {
            weapon.state = WeaponState::Idle;
            weapon.swingTimer = 0.0f;
        }
    }
    else
    {
        // Smooth return to idle pose
        float returnSpeed = 15.0f;
        weapon.swingOffset = Lerp(weapon.swingOffset, 0.0f, dt * returnSpeed);
        weapon.verticalSwingOffset = Lerp(weapon.verticalSwingOffset, 0.0f, dt * returnSpeed);
        weapon.horizontalSwingOffset = Lerp(weapon.horizontalSwingOffset, 0.0f, dt * returnSpeed);
        weapon.attackYawDeg = Lerp(weapon.attackYawDeg, 0.0f, dt * returnSpeed);
        weapon.attackRollDeg = Lerp(weapon.attackRollDeg, 0.0f, dt * returnSpeed);
    }
}

void DrawWeapon(const Weapon &weapon, const Camera3D &camera)
{
    // Camera basis and view rotation inversion
    Matrix lookAt = MatrixLookAt(camera.position, camera.target, (Vector3){0.0f, 1.0f, 0.0f});
    Matrix swordRotation = MatrixInvert(lookAt);
    Quaternion q = QuaternionFromMatrix(swordRotation);

    Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, (Vector3){0.0f, 1.0f, 0.0f}));
    Vector3 camUp = (Vector3){0.0f, 1.0f, 0.0f};

    // Composite offset position in view-space
    float blendedForward = weapon.forwardOffset + weapon.swingOffset;
    float blendedSide = weapon.sideOffset + weapon.horizontalSwingOffset;
    float blendedVertical = weapon.verticalOffset + weapon.verticalSwingOffset;

    Vector3 swordPos = camera.position;
    swordPos = Vector3Add(swordPos, Vector3Scale(camForward, blendedForward));
    swordPos = Vector3Add(swordPos, Vector3Scale(camRight, blendedSide));
    swordPos = Vector3Add(swordPos, Vector3Scale(camUp, blendedVertical));

    // Orientation matrices matching Marooned's pose alignment
    Matrix cameraRot = QuaternionToMatrix(q);
    Matrix baseRoll = MatrixRotateZ(weapon.baseRollDeg * DEG2RAD);
    Matrix attackYaw = MatrixRotateY(weapon.attackYawDeg * DEG2RAD);
    Matrix attackRoll = MatrixRotateZ(weapon.attackRollDeg * DEG2RAD);

    Matrix extraRot = MatrixMultiply(baseRoll, attackYaw);
    extraRot = MatrixMultiply(attackRoll, extraRot);
    Matrix finalRot = MatrixMultiply(extraRot, cameraRot);

    // Decompose final matrix to axis-angle for DrawModelEx
    Quaternion finalQ = QuaternionFromMatrix(finalRot);
    float finalAngle = 2.0f * acosf(Clamp(finalQ.w, -1.0f, 1.0f));
    float finalAngleDeg = finalAngle * RAD2DEG;
    float finalSinTheta = sqrtf(fmaxf(0.0f, 1.0f - finalQ.w * finalQ.w));

    Vector3 finalAxis = (finalSinTheta < 0.001f)
        ? (Vector3){1.0f, 0.0f, 0.0f}
        : (Vector3){finalQ.x / finalSinTheta, finalQ.y / finalSinTheta, finalQ.z / finalSinTheta};

    DrawModelEx(weapon.model, swordPos, finalAxis, finalAngleDeg, weapon.scale, WHITE);
    DrawModelWiresEx(weapon.model, swordPos, finalAxis, finalAngleDeg, weapon.scale, DARKGRAY);
}

void UnloadWeapon(Weapon &weapon)
{
    UnloadModel(weapon.model);
}
