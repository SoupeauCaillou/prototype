#include "ParatrooperSystem.h"

#include "base/PlacementHelper.h"

#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

#include "util/IntersectionUtil.h"

#include <glm/glm.hpp>

INSTANCE_IMPL(ParatrooperSystem);

ParatrooperSystem::ParatrooperSystem() : ComponentSystemImpl <ParatrooperComponent>("Paratrooper") {
}

void ParatrooperSystem::DoUpdate(float) {
	FOR_EACH_ENTITY_COMPONENT(Paratrooper, e, pc)
		if (!pc->landed) {
			if (IntersectionUtil::pointRectangle(glm::vec2(TRANSFORM(e)->position.x, -PlacementHelper::ScreenHeight/2.f), TRANSFORM(e)->position, TRANSFORM(e)->size)) {
				if (glm::abs(PHYSICS(e)->linearVelocity.y) > 1.5f){
					LOGW("Soldier "<< e << " crashed");
					RENDERING(e)->color = Color(1, 0, 0);
				}
				else {
					LOGW("Soldier "<< e << " landed");
					RENDERING(e)->color = Color(0, 1, 0);
				}
				pc->landed = true;
				std::cout << PHYSICS(e)->linearVelocity.x << " " << PHYSICS(e)->linearVelocity << std::endl;
				PHYSICS(e)->linearVelocity = glm::vec2(0.f);
				PHYSICS(e)->mass = 0;
			}
		}
	}
}

#if SAC_INGAME_EDITORS
void ParatrooperSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    ParatrooperComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
