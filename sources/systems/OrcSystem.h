#pragma once

#include "systems/System.h"

namespace AttackState {
	enum Enum {
		Idle,
		Attacking,
		BackToGuard
	};
}

struct OrcComponent {
	Entity weapon;
	float level;
	float levelUp;
	float speed;
	float maxSpeed;
	float attackSpeed;
	AttackState::Enum atkState;
	float atkAccum;
	int facing;
};

#define theOrcSystem OrcSystem::GetInstance()
#define ORC(actor) theOrcSystem.Get(actor)
UPDATABLE_SYSTEM(Orc)

};
