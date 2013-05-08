#pragma once

#include "systems/System.h"

struct ParachuteComponent {
	ParachuteComponent(): frottement(1.f), destroyedLeft(false), destroyedRight(false) {};
	float frottement;

    bool destroyedLeft, destroyedRight;
};

#define theParachuteSystem ParachuteSystem::GetInstance()
#define PARACHUTE(e) theParachuteSystem.Get(e)

UPDATABLE_SYSTEM(Parachute)

public:
    void destroyParachute(Entity parachute);
};
