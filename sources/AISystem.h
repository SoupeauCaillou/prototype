#pragma once

#include "systems/System.h"

namespace AI {
    enum Enum {
        Idle,
        ReceiveBall,
    };
};

struct AIComponent {
    AIComponent() {
        state = AI::Idle;
    }
    AI::Enum state;
};

#define theAISystem AISystem::GetInstance()
#define AI(e) theAISystem.Get(e)

UPDATABLE_SYSTEM(AI)

};
