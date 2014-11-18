#pragma once

#include "systems/System.h"

struct UnitComponent {
    UnitComponent() : index(-1), alive(true) {}

    int index;
    Entity body, head, weapon[2], hitzone;
    bool alive;
};

#define theUnitSystem UnitSystem::GetInstance()
#if SAC_DEBUG
#define UNIT(e) theUnitSystem.Get(e,true,__FILE__,__LINE__)
#else
#define UNIT(e) theUnitSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Unit)
};
