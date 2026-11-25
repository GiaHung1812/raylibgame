#pragma once

#include <raylib.h>

enum class WeaponState
{
    Idle,
    Attacking
};

struct Weapon
{
    WeaponState state = WeaponState::Idle;
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

    float attackYawDeg = 0.0f;
    float attackRollDeg = 0.0f;

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
void DrawWeapon(const Weapon &weapon, const Camera3D &camera);
void UnloadWeapon(Weapon &weapon);
