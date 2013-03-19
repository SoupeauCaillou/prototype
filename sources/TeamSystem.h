#pragma once

#include "systems/System.h"
#include "base/Color.h"

struct TeamComponent {
    Color color;
};

#define theTeamSystem TeamSystem::GetInstance()
#define TEAM(e) theTeamSystem.Get(e)

UPDATABLE_SYSTEM(Team)

};
