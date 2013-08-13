#include "PlayerSystem.h"

INSTANCE_IMPL(PlayerSystem);

PlayerSystem::PlayerSystem() : ComponentSystemImpl <PlayerComponent>("Player") {
	PlayerComponent pc;
	componentSerializer.add(new Property<int>("id", OFFSET(id, pc)));
    componentSerializer.add(new Property<int>("score", OFFSET(score, pc)));
}

void PlayerSystem::DoUpdate(float) {
	// FOR_EACH_ENTITY_COMPONENT(Plane, e, pc)
	// }
}
