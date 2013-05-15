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
	InputComponent() : action(Action::None) {
        memset(&SpawnParams, 0, sizeof(SpawnParams));
        memset(&OpenParachuteParams, 0, sizeof(OpenParachuteParams));
        memset(&FireParams, 0, sizeof(FireParams));
    }

	Action::Enum action;
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

#define theInputSystem InputSystem::GetInstance()
#define INPUT(e) theInputSystem.Get(e)

UPDATABLE_SYSTEM(Input)
};
