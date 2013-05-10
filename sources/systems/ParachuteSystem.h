#pragma once

#include "systems/System.h"

struct ParachuteComponent {
	ParachuteComponent(): frottement(1.f) {}
	float frottement;

    Entity fils;
    std::vector<glm::vec2> damages;
    std::vector<Entity> holes;
};

#define theParachuteSystem ParachuteSystem::GetInstance()
#define PARACHUTE(e) theParachuteSystem.Get(e)

UPDATABLE_SYSTEM(Parachute)
};
