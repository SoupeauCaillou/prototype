#pragma once

#include "systems/System.h"
#include "base/Frequency.h"

struct AIComponent {
    AIComponent() : decisionPerSecond(1), accum(0), aimingNoise(0.0f) {}
    float decisionPerSecond;
    float accum;
    float aimingNoise;
};

#define theAISystem AISystem::GetInstance()
#define AI(e) theAISystem.Get(e)

UPDATABLE_SYSTEM(AI)
};
