#pragma once

#include "systems/System.h"


struct SwordManComponent {
    SwordManComponent() {
        hands[0] = hands[1] = 0;
    }
    Entity hands[2];
};

#define theSwordManSystem SwordManSystem::GetInstance()
#define SWORD_MAN(e) theSwordManSystem.Get(e)

UPDATABLE_SYSTEM(SwordMan)

};
