#include "weapon.h"
#include <raymath.h>

void InitWeapon(Weapon &weapon)
{
    weapon.state = WeaponState::Idle;
    weapon.forwardOffset = 0.42f;
    weapon.sideOffset = 0.28f;
    weapon.verticalOffset = -0.45f;
    weapon.baseRollDeg = 30.0f;
    weapon.scale = {0.022f, 0.022f, 0.022f};

    weapon.swingTimer = 0.0f;
    weapon.swingOffset = 0.0f;
    weapon.horizontalSwingOffset = 0.0f;
    weapon.verticalSwingOffset = 0.0f;
    weapon.attackYawDeg = 0.0f;
    weapon.attackRollDeg = 0.0f;

    weapon.bobTimer = 0.0f;
    weapon.bobVertical = 0.0f;
    weapon.bobSide = 0.0f;

    weapon.hitTriggered = false;
    weapon.attackId = 0;

    weapon.model = LoadModel("assets/sword.glb");
}

void StartWeaponSwing(Weapon &weapon)
{
    if (weapon.state != WeaponState::Idle)
        return;

    weapon.state = WeaponState::Attacking;
    weapon.swingTimer = 0.0f;
    weapon.hitTriggered = false;
    weapon.attackId++;
}

static void UpdateWeaponBobbing(Weapon &weapon, bool isMoving, float dt)
{
    if (isMoving)
    {
        weapon.bobTimer += dt * 10.0f;
        weapon.bobVertical = sinf(weapon.bobTimer) * 0.015f;
        weapon.bobSide = sinf(weapon.bobTimer * 0.5f) * 0.012f;
        return;
    }

    weapon.bobTimer = 0.0f;
    weapon.bobVertical = Lerp(weapon.bobVertical, 0.0f, dt * 8.0f);
    weapon.bobSide = Lerp(weapon.bobSide, 0.0f, dt * 8.0f);
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

bool UpdateWeapon(Weapon &weapon, bool isMoving, float dt)
{
    UpdateWeaponBobbing(weapon, isMoving, dt);

    bool shouldCheckHit = false;

    if (weapon.state == WeaponState::Attacking)
    {
        weapon.swingTimer += dt;
        float t = Clamp(weapon.swingTimer / weapon.swingDuration, 0.0f, 1.0f);

        UpdateRightSlashMotion(weapon, t);

        // Trigger hit detection once inside active swing window
        if (!weapon.hitTriggered &&
            weapon.swingTimer >= weapon.hitWindowStart &&
            weapon.swingTimer <= weapon.hitWindowEnd)
        {
            weapon.hitTriggered = true;
            shouldCheckHit = true;
        }

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

    return shouldCheckHit;
}

void DrawWeapon(const Weapon &weapon, const Camera3D &camera)
{
    // Camera basis and view rotation inversion
    Matrix lookAt = MatrixLookAt(camera.position, camera.target, (Vector3){0.0f, 1.0f, 0.0f});
    Matrix swordRotation = MatrixInvert(lookAt);
    Quaternion q = QuaternionFromMatrix(swordRotation);

    Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, (Vector3){0.0f, 1.0f, 0.0f}));
    Vector3 camUp = Vector3CrossProduct(camRight, camForward);

    // Composite offset position in view-space
    float blendedForward = weapon.forwardOffset + weapon.swingOffset;
    float blendedSide = weapon.sideOffset + weapon.horizontalSwingOffset + weapon.bobSide;
    float blendedVertical = weapon.verticalOffset + weapon.verticalSwingOffset + weapon.bobVertical;

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
}

void UnloadWeapon(Weapon &weapon)
{
    UnloadModel(weapon.model);
}
