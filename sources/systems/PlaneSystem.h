#pragma once

#include "base/Frequency.h"

#include "systems/System.h"

#include <glm/glm.hpp>

struct PlaneComponent {
	PlaneComponent():paratrooper(1), timeBetweenJumps(1.f){};
	glm::vec2 speed;
	int paratrooper;
	Frequency<float> timeBetweenJumps;
};

#define thePlaneSystem PlaneSystem::GetInstance()
#define PLANE(e) thePlaneSystem.Get(e)

UPDATABLE_SYSTEM(Plane)

	public:
		void setWorldSize(const glm::vec2 ws);
		Entity paratrooperJump(Entity& plane);

	private:
		glm::vec2 worldSize;
};
