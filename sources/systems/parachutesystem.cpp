#include "parachutesystem.h"

#include "systems/PhysicsSystem.h"

INSTANCE_IMPL(ParachuteSystem);

ParachuteSystem::ParachuteSystem() : ComponentSystemImpl <ParachuteComponent>("Parachute") {

}

void ParachuteSystem::DoUpdate(float dt) {
	FOR_EACH_ENTITY_COMPONENT(Parachute, e, pc)
		if (pc->enable) {
			PhysicsComponent *phc = PHYSICS(e);

			float force = 1/2 * pc->frottement * phc->linearVelocity.y * phc->linearVelocity.y;
			phc->forces.push_back(std::make_pair(Force(glm::vec2(0.f, -force), glm::vec2(0.f)), 1/60));
		}
	}
}

#if SAC_INGAME_EDITORS
void ParachuteSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ParachuteComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif