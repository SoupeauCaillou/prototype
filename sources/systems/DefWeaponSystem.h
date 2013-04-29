#pragma once

#include <glm/glm.hpp>

#include "systems/System.h"


struct DefWeaponComponent {
	DefWeaponComponent(): ellipseParam(1.0f), ellipseAngleRange(0) {}

	glm::vec2 target;
	unsigned action;
	glm::vec2 ellipseParam, ellipseAngleRange;
	float maxAngularSpeed;
};

#define theDefWeaponSystem DefWeaponSystem::GetInstance()
#define DEF_WEAPON(e) theDefWeaponSystem.Get(e)

UPDATABLE_SYSTEM(DefWeapon)

};
