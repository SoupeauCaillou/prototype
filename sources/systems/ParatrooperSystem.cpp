#include "ParatrooperSystem.h"

INSTANCE_IMPL(ParatrooperSystem);

ParatrooperSystem::ParatrooperSystem() : ComponentSystemImpl <ParatrooperComponent>("Paratrooper") {

}

void ParatrooperSystem::DoUpdate(float) {
	//FOR_EACH_ENTITY_COMPONENT(Paratroopera, e, pc)
	//
	//}
}

#if SAC_INGAME_EDITORS
void ParatrooperSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    ParatrooperComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
