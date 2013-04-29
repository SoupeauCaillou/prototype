#pragma once

#include <glm/glm.hpp>

#include "systems/System.h"


struct SwordComponent {
	SwordComponent(): ellipseParam(1.0f), ellipseAngleRange(0) {}

	glm::vec2 target;
	unsigned action;
	glm::vec2 ellipseParam, ellipseAngleRange;
	float maxAngularSpeed;
};

#define theSwordSystem SwordSystem::GetInstance()
#define SWORD(e) theSwordSystem.Get(e)

UPDATABLE_SYSTEM(Sword)

};
