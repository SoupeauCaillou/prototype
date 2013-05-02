#pragma once

#include "systems/System.h"

struct ParachuteComponent {
	float frottement;
	bool enable;
};

#define theParachuteSystem ParachuteSystem::GetInstance()
#define PARACHUTE(e) theParachuteSystem.Get(e)

UPDATABLE_SYSTEM(Parachute)
};
