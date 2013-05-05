#include "ParatrooperSystem.h"

INSTANCE_IMPL(ParatrooperSystem);

ParatrooperSystem::ParatrooperSystem() : ComponentSystemImpl <ParatrooperComponent>("Paratrooper") {

}

void ParatrooperSystem::DoUpdate(float dt) {
	//FOR_EACH_ENTITY_COMPONENT(Paratroopera, e, pc)
	//
	//}
}

#if SAC_INGAME_EDITORS
void ParatrooperSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ParatrooperComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
