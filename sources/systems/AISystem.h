#pragma once

#include "systems/System.h"

struct AIComponent {
    AIComponent() : minAngle(0), maxAngle(0), rotationSpeed(1), _targetAngle(0) {pauses.t1 = pauses.t2 = 0.0f; _pauseAccum = 0.0f;}
    float minAngle, maxAngle;
    float rotationSpeed;
    Interval<float> pauses;

    float _targetAngle;
    float _pauseAccum;
};

#define theAISystem AISystem::GetInstance()
#if SAC_DEBUG
#define AI(e) theAISystem.Get(e,true,__FILE__,__LINE__)
#else
#define AI(e) theAISystem.Get(e)
#endif

UPDATABLE_SYSTEM(AI)
};
