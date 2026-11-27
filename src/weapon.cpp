#include "weapon.h"
#include <raymath.h>

void ResetWeapon(Weapon &weapon)
{
    weapon.state = WeaponState::Idle;
    weapon.currentAttack = SwordAttackType::RightSlash;
    weapon.forwardOffset = 0.46f;
    weapon.sideOffset = 0.30f;
    weapon.verticalOffset = -0.36f;
    weapon.basePitchDeg = -14.0f;
    weapon.baseYawDeg = 14.0f;
    weapon.baseRollDeg = 6.0f;
    weapon.scale = {0.017f, 0.017f, 0.017f};

    weapon.swingTimer = 0.0f;
    weapon.swingDuration = 0.35f;
    weapon.swingOffset = 0.0f;
    weapon.horizontalSwingOffset = 0.0f;
    weapon.verticalSwingOffset = 0.0f;

    weapon.attackForwardOffset = 0.0f;
    weapon.attackSideOffset = 0.0f;
    weapon.attackVerticalOffset = 0.0f;
    weapon.attackYawDeg = 0.0f;
    weapon.attackRollDeg = 0.0f;

    weapon.comboIndex = 0;
    weapon.comboTimer = 0.0f;
    weapon.bobTimer = 0.0f;
    weapon.bobVertical = 0.0f;
    weapon.bobSide = 0.0f;

    weapon.hitTriggered = false;
}

void InitWeapon(Weapon &weapon)
{
    ResetWeapon(weapon);
    weapon.attackId = 0;
    weapon.model = LoadModel("assets/sword.glb");
}

static SwordAttackType GetComboAttack(int comboIndex)
{
    switch (comboIndex)
    {
    case 0:
        return SwordAttackType::RightSlash;
    case 1:
        return SwordAttackType::LeftSlash;
    case 2:
        return SwordAttackType::Stab;
    default:
        return SwordAttackType::RightSlash;
    }
}

void StartWeaponSwing(Weapon &weapon)
{
    if (weapon.state != WeaponState::Idle)
        return;

    weapon.currentAttack = GetComboAttack(weapon.comboIndex);
    weapon.comboIndex = (weapon.comboIndex + 1) % 3;
    weapon.comboTimer = 0.0f;

    weapon.state = WeaponState::Attacking;
    weapon.swingTimer = 0.0f;
    weapon.hitTriggered = false;
    weapon.attackId++;
}

float GetWeaponDamage(const Weapon &weapon)
{
    switch (weapon.currentAttack)
    {
    case SwordAttackType::RightSlash:
        return 25.0f;
    case SwordAttackType::LeftSlash:
        return 25.0f;
    case SwordAttackType::Stab:
        return 50.0f;
    }
    return 25.0f;
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
    float tFast = Clamp(t * 2.0f, 0.0f, 1.0f);
    float arc = sinf(tFast * (PI * 0.6f));

    weapon.swingOffset = arc * 0.15f;
    weapon.verticalSwingOffset = -arc * 0.25f;
    weapon.horizontalSwingOffset = -arc * 0.25f;

    weapon.attackForwardOffset = 0.0f;
    weapon.attackSideOffset = 0.0f;
    weapon.attackVerticalOffset = 0.0f;

    weapon.attackYawDeg = arc * -35.0f;
    weapon.attackRollDeg = arc * 25.0f;
}

static void UpdateLeftSlashMotion(Weapon &weapon, float t)
{
    float tFast = Clamp(t * 1.8f, 0.0f, 1.0f);
    float arc = sinf(tFast * (PI * 0.6f));

    weapon.swingOffset = arc * 0.15f;
    weapon.verticalSwingOffset = -arc * 0.22f;
    weapon.horizontalSwingOffset = arc * 0.28f;

    // Shift starting pose toward left side for backhand sweep
    weapon.attackForwardOffset = 0.08f;
    weapon.attackSideOffset = -0.32f;
    weapon.attackVerticalOffset = 0.02f;

    weapon.attackYawDeg = arc * 30.0f;
    weapon.attackRollDeg = -45.0f;
}

static void UpdateStabMotion(Weapon &weapon, float t)
{
    // Three-phase thrust: quick extend, brief hold, smooth retract
    const float extendEnd = 0.30f;
    const float holdEnd = 0.75f;
    float jab = 0.0f;

    if (t < extendEnd)
    {
        float p = t / extendEnd;
        jab = sinf(p * PI * 0.5f);
    }
    else if (t < holdEnd)
    {
        jab = 1.0f;
    }
    else
    {
        float p = (t - holdEnd) / (1.0f - holdEnd);
        jab = 1.0f - p;
    }

    weapon.swingOffset = jab * 0.42f;
    weapon.horizontalSwingOffset = 0.0f;
    weapon.verticalSwingOffset = 0.05f * jab;

    weapon.attackForwardOffset = 0.10f;
    weapon.attackSideOffset = -0.10f;
    weapon.attackVerticalOffset = 0.04f;

    weapon.attackYawDeg = 0.0f;
    weapon.attackRollDeg = -15.0f * jab;
}

bool UpdateWeapon(Weapon &weapon, bool isMoving, float dt)
{
    UpdateWeaponBobbing(weapon, isMoving, dt);

    bool shouldCheckHit = false;
    weapon.comboTimer += dt;

    if (weapon.state == WeaponState::Attacking)
    {
        weapon.swingTimer += dt;
        float t = Clamp(weapon.swingTimer / weapon.swingDuration, 0.0f, 1.0f);

        switch (weapon.currentAttack)
        {
        case SwordAttackType::RightSlash:
            UpdateRightSlashMotion(weapon, t);
            break;
        case SwordAttackType::LeftSlash:
            UpdateLeftSlashMotion(weapon, t);
            break;
        case SwordAttackType::Stab:
            UpdateStabMotion(weapon, t);
            break;
        }

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
        if (weapon.comboTimer > weapon.comboResetTime)
            weapon.comboIndex = 0;

        // Smooth return to idle pose
        bool comboAlive = (weapon.comboTimer <= weapon.comboResetTime && weapon.comboIndex > 0);
        float returnSpeed = comboAlive ? 10.0f : 18.0f;

        weapon.swingOffset = Lerp(weapon.swingOffset, 0.0f, dt * returnSpeed);
        weapon.verticalSwingOffset = Lerp(weapon.verticalSwingOffset, 0.0f, dt * returnSpeed);
        weapon.horizontalSwingOffset = Lerp(weapon.horizontalSwingOffset, 0.0f, dt * returnSpeed);
        weapon.attackForwardOffset = Lerp(weapon.attackForwardOffset, 0.0f, dt * returnSpeed);
        weapon.attackSideOffset = Lerp(weapon.attackSideOffset, 0.0f, dt * returnSpeed);
        weapon.attackVerticalOffset = Lerp(weapon.attackVerticalOffset, 0.0f, dt * returnSpeed);
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
    float blendedForward = weapon.forwardOffset + weapon.swingOffset + weapon.attackForwardOffset;
    float blendedSide = weapon.sideOffset + weapon.horizontalSwingOffset + weapon.attackSideOffset + weapon.bobSide;
    float blendedVertical = weapon.verticalOffset + weapon.verticalSwingOffset + weapon.attackVerticalOffset + weapon.bobVertical;

    Vector3 swordPos = camera.position;
    swordPos = Vector3Add(swordPos, Vector3Scale(camForward, blendedForward));
    swordPos = Vector3Add(swordPos, Vector3Scale(camRight, blendedSide));
    swordPos = Vector3Add(swordPos, Vector3Scale(camUp, blendedVertical));

    // Orientation matrices aligning the GLB model to view space
    Matrix cameraRot = QuaternionToMatrix(q);
    Matrix baseRoll = MatrixRotateZ(weapon.baseRollDeg * DEG2RAD);
    Matrix basePitch = MatrixRotateX(weapon.basePitchDeg * DEG2RAD);
    Matrix baseYaw = MatrixRotateY(weapon.baseYawDeg * DEG2RAD);
    Matrix attackYaw = MatrixRotateY(weapon.attackYawDeg * DEG2RAD);
    Matrix attackRoll = MatrixRotateZ(weapon.attackRollDeg * DEG2RAD);

    Matrix extraRot = MatrixMultiply(baseRoll, basePitch);
    extraRot = MatrixMultiply(extraRot, baseYaw);
    extraRot = MatrixMultiply(extraRot, attackYaw);
    extraRot = MatrixMultiply(extraRot, attackRoll);
    Matrix finalRot = MatrixMultiply(extraRot, cameraRot);

    Vector3 finalAxis = {1.0f, 0.0f, 0.0f};
    float finalAngleRad = 0.0f;
    QuaternionToAxisAngle(QuaternionFromMatrix(finalRot), &finalAxis, &finalAngleRad);

    DrawModelEx(weapon.model, swordPos, finalAxis, finalAngleRad * RAD2DEG, weapon.scale, WHITE);
}

void UnloadWeapon(Weapon &weapon)
{
    UnloadModel(weapon.model);
}
