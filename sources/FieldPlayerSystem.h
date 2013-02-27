#pragma once

#include "systems/System.h"

#define UP     (1 << 0)
#define DOWN   (1 << 1)
#define LEFT   (1 << 2)
#define RIGHT  (1 << 3)
#define SPRINT (1 << 4)
#define PASS   (1 << 5)

struct FieldPlayerComponent {
    FieldPlayerComponent() {
        speed = 8;
        accel = 80;
        friction = 30;
        maxForce = 1000;
        ballSpeedDecrease = 0.7;
        sprintBoost = 1.2;
        keyPresses = 0;
    }
    float speed, accel, friction, maxForce, ballSpeedDecrease, sprintBoost;
    Entity ballContact;
    unsigned int keyPresses;
};

#define theFieldPlayerSystem FieldPlayerSystem::GetInstance()
#define FIELD_PLAYER(e) theFieldPlayerSystem.Get(e)

UPDATABLE_SYSTEM(FieldPlayer)

};
