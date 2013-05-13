#pragma once

#include "systems/System.h"

struct ParatrooperComponent {
	ParatrooperComponent():landed(false), parachuteOpened(false), dead(false), owner(0), parachute(0) {};
	bool landed, parachuteOpened;
	bool dead;
	Entity owner, parachute;
};

#define theParatrooperSystem ParatrooperSystem::GetInstance()
#define PARATROOPER(e) theParatrooperSystem.Get(e)

UPDATABLE_SYSTEM(Paratrooper)
};
