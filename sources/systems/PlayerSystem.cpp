#include "PlayerSystem.h"

INSTANCE_IMPL(PlayerSystem);

PlayerSystem::PlayerSystem() : ComponentSystemImpl <PlayerComponent>("Player") {
	PlayerComponent pc;
	componentSerializer.add(new Property<int>("id", OFFSET(id, pc)));
}

void PlayerSystem::DoUpdate(float) {
	// FOR_EACH_ENTITY_COMPONENT(Plane, e, pc)
	// }
}

#if SAC_INGAME_EDITORS
void PlayerSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    PlayerComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
