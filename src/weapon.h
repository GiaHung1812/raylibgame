#pragma once

#include <raylib.h>

enum class WeaponState
{
    Idle,
    Attacking
};

enum class SwordAttackType
{
    RightSlash,
    LeftSlash,
    Stab
};

struct Weapon
{
    WeaponState state = WeaponState::Idle;
    SwordAttackType currentAttack = SwordAttackType::RightSlash;
    Model model;
    Vector3 scale = {0.022f, 0.022f, 0.022f};

    // Rest pose offsets
    float forwardOffset = 0.42f;
    float sideOffset = 0.28f;
    float verticalOffset = -0.45f;
    float baseRollDeg = 30.0f;

    // Swing animation parameters
    float swingTimer = 0.0f;
    float swingDuration = 0.35f;

    float swingOffset = 0.0f;
    float horizontalSwingOffset = 0.0f;
    float verticalSwingOffset = 0.0f;

    float attackForwardOffset = 0.0f;
    float attackSideOffset = 0.0f;
    float attackVerticalOffset = 0.0f;

    float attackYawDeg = 0.0f;
    float attackRollDeg = 0.0f;

    // Combo chaining state
    int comboIndex = 0;
    float comboTimer = 0.0f;
    float comboResetTime = 0.85f;

    // Viewmodel bobbing state
    float bobTimer = 0.0f;
    float bobVertical = 0.0f;
    float bobSide = 0.0f;

    // Melee hit window state
    float hitWindowStart = 0.08f;
    float hitWindowEnd = 0.20f;
    bool hitTriggered = false;
    int attackId = 0;
};

void InitWeapon(Weapon &weapon);
void StartWeaponSwing(Weapon &weapon);
bool UpdateWeapon(Weapon &weapon, bool isMoving, float dt);
float GetWeaponDamage(const Weapon &weapon);
void DrawWeapon(const Weapon &weapon, const Camera3D &camera);
void UnloadWeapon(Weapon &weapon);
