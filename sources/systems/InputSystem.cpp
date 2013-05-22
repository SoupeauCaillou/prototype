#include "InputSystem.h"

#include "systems/ZSQDSystem.h"

#include <glm/gtx/norm.hpp>

INSTANCE_IMPL(InputSystem);

InputSystem::InputSystem() : ComponentSystemImpl <InputComponent>("Input") {
	InputComponent tc;
    componentSerializer.add(new Property<glm::vec2>("direction", OFFSET(direction, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<bool>("lateral_move", OFFSET(lateralMove, tc), 0));
}

void InputSystem::DoUpdate(float) {
	FOR_EACH_ENTITY_COMPONENT(Input, entity, ic)
		if (glm::length2(ic->direction) > 0.001) {
			ZSQD(entity)->directions.push_back(ic->direction);
			ZSQD(entity)->lateralMove = ic->lateralMove;
		}
	}
}