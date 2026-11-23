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
    Vector3 scale = {0.8f, 0.8f, 0.8f};

    // Rest pose offsets
    float forwardOffset = 0.55f;
    float sideOffset = 0.28f;
    float verticalOffset = -0.30f;
    float baseRollDeg = -15.0f;

    // Swing animation parameters
    float swingTimer = 0.0f;
    float swingDuration = 0.35f;

    float swingOffset = 0.0f;
    float horizontalSwingOffset = 0.0f;
    float verticalSwingOffset = 0.0f;

    float attackYawDeg = 0.0f;
    float attackRollDeg = 0.0f;
};

void InitWeapon(Weapon &weapon);
void UpdateWeapon(Weapon &weapon, float dt);
void DrawWeapon(const Weapon &weapon, const Camera3D &camera);
void UnloadWeapon(Weapon &weapon);
