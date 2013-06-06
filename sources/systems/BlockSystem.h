#pragma once

#include "systems/System.h"

struct BlockComponent {
    BlockComponent() : doubleFace(false) {}

    // is the wall visible from the 2 sides?
    bool doubleFace;
};

#define theBlockSystem BlockSystem::GetInstance()
#define BLOCK(e) theBlockSystem.Get(e)

UPDATABLE_SYSTEM(Block)
};
