#pragma once

#include "systems/System.h"

struct BlockComponent {
};

#define theBlockSystem BlockSystem::GetInstance()
#define BLOCK(e) theBlockSystem.Get(e)

UPDATABLE_SYSTEM(Block)
    public:
        void CleanEntities();
};
