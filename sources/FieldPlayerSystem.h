#pragma once

#include "systems/System.h"

#define UP    (1 << 0)
#define DOWN  (1 << 1)
#define LEFT  (1 << 2)
#define RIGHT (1 << 3)

struct FieldPlayerComponent {
    FieldPlayerComponent() {
        speed = 8;
        accel = 80;
        friction = 20;
        maxForce = 1000;
        ballOwner = false;
    }
    float speed, accel, friction, maxForce;
    Entity ballContact;
    unsigned int keyPresses;
    bool ballOwner;
};

#define theFieldPlayerSystem FieldPlayerSystem::GetInstance()
#define FIELD_PLAYER(e) theFieldPlayerSystem.Get(e)

UPDATABLE_SYSTEM(FieldPlayer)

};
