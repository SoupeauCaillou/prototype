#pragma once

#include "systems/System.h"

struct ParatrooperComponent {
	ParatrooperComponent():landed(false) {};
	bool landed;
};

#define theParatrooperSystem ParatrooperSystem::GetInstance()
#define PARATROOPER(e) theParatrooperSystem.Get(e)

UPDATABLE_SYSTEM(Paratrooper)
};
