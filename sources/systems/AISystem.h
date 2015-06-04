#pragma once

#include "systems/System.h"
#include "base/Interval.h"

namespace State
{
    enum Enum {
        Idle,
        Firing,
    };
}

struct AIComponent {
    AIComponent() : minAngle(0), maxAngle(0), rotationSpeed(1), state(State::Idle), _targetAngle(0), _lastSeenAccum(0) {pauses.t1 = pauses.t2 = 0.0f; _pauseAccum = 0.0f;}
    float minAngle, maxAngle;
    float rotationSpeed;
    Interval<float> pauses;
    State::Enum state;

    float _targetAngle;
    float _pauseAccum;
    float _lastSeenAccum;
};

#define theAISystem AISystem::GetInstance()
#if SAC_DEBUG
#define AI(e) theAISystem.Get(e,true,__FILE__,__LINE__)
#else
#define AI(e) theAISystem.Get(e)
#endif

UPDATABLE_SYSTEM(AI)
};
