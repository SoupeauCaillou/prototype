#include "dcasystem.h"

INSTANCE_IMPL(DcaSystem);

DcaSystem::DcaSystem() : ComponentSystemImpl <DcaComponent>("Dca") {

}

void DcaSystem::DoUpdate(float dt) {
	// FOR_EACH_ENTITY_COMPONENT(Dca, e, pc)
		
	// }
}

#if SAC_INGAME_EDITORS
void DcaSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    DcaComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif