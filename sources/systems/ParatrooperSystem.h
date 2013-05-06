#pragma once

#include "systems/System.h"

struct ParatrooperComponent {
	ParatrooperComponent():weight(1), landed(false) {};
	float weight;
	bool landed;
};

#define theParatrooperSystem ParatrooperSystem::GetInstance()
#define PARATROOPER(e) theParatrooperSystem.Get(e)

UPDATABLE_SYSTEM(Paratrooper)
};
