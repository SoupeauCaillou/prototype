#pragma once

#include "systems/System.h"

namespace Action {
	enum Enum {
		None,
		Spawn,
		OpenParachute,
		Fire,
	};
}

struct InputComponent {
	InputComponent() {}

	Action::Enum action;
	union {
		struct {
			Entity plane;
		} SpawnParams;
		struct {
			Entity paratrooper;
		} OpenParachuteParams;
		struct {
			Entity dca;
			glm::vec2 aim;
		} FireParams;
	};
};

#define theInputSystem InputSystem::GetInstance()
#define INPUT(e) theInputSystem.Get(e)

UPDATABLE_SYSTEM(Input)
};
