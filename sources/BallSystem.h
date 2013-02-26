#pragma once

#include "systems/System.h"

struct BallComponent {
    BallComponent() {
        friction = -10;
        owner = 0;
    }
    float friction;
    Entity owner;
};

#define theBallSystem BallSystem::GetInstance()
#define BALL(e) theBallSystem.Get(e)

UPDATABLE_SYSTEM(Ball)

};
