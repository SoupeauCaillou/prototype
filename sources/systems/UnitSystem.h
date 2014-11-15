#pragma once

#include "systems/System.h"

struct UnitComponent {
    Entity body, head, weapon, hitzone;
};

#define theUnitSystem UnitSystem::GetInstance()
#if SAC_DEBUG
#define UNIT(e) theUnitSystem.Get(e,true,__FILE__,__LINE__)
#else
#define UNIT(e) theUnitSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Unit)
};
