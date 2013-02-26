#pragma once

#include "systems/System.h"

struct BallComponent {
    BallComponent() {
        friction = -10;
    }
    float friction;
};

#define theBallSystem BallSystem::GetInstance()
#define BALL(e) theBallSystem.Get(e)

UPDATABLE_SYSTEM(Ball)

};
