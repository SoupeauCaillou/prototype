#include "ParachuteSystem.h"

#include "systems/PhysicsSystem.h"

INSTANCE_IMPL(ParachuteSystem);

ParachuteSystem::ParachuteSystem() : ComponentSystemImpl <ParachuteComponent>("Parachute") {
	ParachuteComponent pc;
    componentSerializer.add(new Property<float>("frottement", OFFSET(frottement, pc), 0.001));
}

void ParachuteSystem::DoUpdate(float dt) {
	FOR_EACH_ENTITY_COMPONENT(Parachute, e, pc)
		PhysicsComponent *phc = PHYSICS(e);

		float force = 0.5f * pc->frottement * phc->linearVelocity.y * phc->linearVelocity.y;
		phc->forces.push_back(std::make_pair(Force(glm::vec2(0.f, force), glm::vec2(1.f, 0.f)), dt));
	}
}

#if SAC_INGAME_EDITORS
void ParachuteSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ParachuteComponent* pc = Get(entity, false);
    if (!pc) return;
	TwAddVarRW(bar, "frottement", TW_TYPE_FLOAT, &pc->frottement, "group=Parachute precision=2 step=0,01");
}
#endif
