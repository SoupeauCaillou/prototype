#pragma once

#include "systems/System.h"

struct ParatrooperComponent {
	ParatrooperComponent():landed(false), dead(false), owner(0) {};
	bool landed;
	bool dead;
	Entity owner;
};

#define theParatrooperSystem ParatrooperSystem::GetInstance()
#define PARATROOPER(e) theParatrooperSystem.Get(e)

UPDATABLE_SYSTEM(Paratrooper)
};
