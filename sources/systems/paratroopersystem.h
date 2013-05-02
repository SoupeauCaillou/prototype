#pragma once

#include "systems/System.h"

struct ParatrooperComponent {
	float weight;	
};

#define theParatrooperSystem ParatrooperSystem::GetInstance()
#define PARATROOPER(e) theParatrooperSystem.Get(e)

UPDATABLE_SYSTEM(Paratrooper)
};
