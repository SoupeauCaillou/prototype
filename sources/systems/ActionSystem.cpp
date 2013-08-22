#include "ActionSystem.h"
#include "OrcSystem.h"

#include "systems/TransformationSystem.h"

INSTANCE_IMPL(ActionSystem);

ActionSystem::ActionSystem() : ComponentSystemImpl<ActionComponent>("Action") {
    ActionComponent ac;
    componentSerializer.add(new EntityProperty("orc", OFFSET(orc, ac)));
}

void ActionSystem::DoUpdate(float dt) {
	for (auto& p: components) {
		const Entity action = p.first;
		auto* ac = p.second;

	}
}
