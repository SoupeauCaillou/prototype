#pragma once

#include "systems/System.h"

struct ActionComponent {
	ActionComponent() : attack(false), move(0) {}
	Entity orc;
	bool attack;
	float move;
};

#define theActionSystem ActionSystem::GetInstance()
#define ACTION(actor) theActionSystem.Get(actor)
UPDATABLE_SYSTEM(Action)

};
