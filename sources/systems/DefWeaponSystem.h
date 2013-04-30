#pragma once

#include <glm/glm.hpp>

#include "systems/System.h"


struct DefWeaponComponent {
	DefWeaponComponent(): active(false), ellipseParam(1.0f), ellipseAngleRange(0), modeTransitionDuration(0) {}

    bool active, attack;
	glm::vec2 target;
	unsigned action;
	glm::vec2 ellipseParam, ellipseAngleRange, attackModeOffset;
	float maxAngularSpeed, modeTransitionDuration;
    float angle;
};

#define theDefWeaponSystem DefWeaponSystem::GetInstance()
#define DEF_WEAPON(e) theDefWeaponSystem.Get(e)

UPDATABLE_SYSTEM(DefWeapon)

private:
    std::map<Entity, Entity> transition;
};
