#pragma once

#include "base/Frequency.h"

#include "systems/System.h"

#include <glm/glm.hpp>

struct PlaneComponent {
	PlaneComponent():paratrooperAvailable(1), paratrooperLimit(1), dropOne(false), timeBetweenJumps(1.f),
	owner(0){};
	glm::vec2 speed;
	int paratrooperAvailable, paratrooperLimit;

	bool dropOne;

	Frequency<float> timeBetweenJumps;

	Entity owner;
};

#define thePlaneSystem PlaneSystem::GetInstance()
#define PLANE(e) thePlaneSystem.Get(e)

UPDATABLE_SYSTEM(Plane)
};
