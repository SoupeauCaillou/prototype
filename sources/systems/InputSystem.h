#pragma once

#include "systems/System.h"

namespace Action {
	enum Enum {

	};
}

struct InputComponent {
	InputComponent() : direction(0.0f), lateralMove(false) {}

	glm::vec2 direction;
	bool lateralMove;
};

#define theInputSystem InputSystem::GetInstance()
#define INPUT(e) theInputSystem.Get(e)

UPDATABLE_SYSTEM(Input)
};