#pragma once

#include "systems/System.h"

#include <glm/glm.hpp>

struct PlaneComponent {
	PlaneComponent():paratrooper(1), timeBetweenJumps(1.f){};
	glm::vec2 speed;
	int paratrooper;
	float timeBetweenJumps;
};

#define thePlaneSystem PlaneSystem::GetInstance()
#define PLANE(e) thePlaneSystem.Get(e)

UPDATABLE_SYSTEM(Plane)

};
