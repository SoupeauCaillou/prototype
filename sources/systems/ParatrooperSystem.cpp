#include "ParatrooperSystem.h"

#include "base/PlacementHelper.h"

#include "systems/AutoDestroySystem.h"
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
					PHYSICS(e)->mass = 0;
					AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;
					AUTO_DESTROY(e)->params.lifetime.freq.value = 1;
				}
				else {
					LOGW("Soldier "<< e << " landed");
					RENDERING(e)->color = Color(0, 1, 0);
					PHYSICS(e)->linearVelocity.y = 0;
				}
				pc->landed = true;
			}
		}
		else {
			TRANSFORM(e)->position.y = (TRANSFORM(e)->size.y-PlacementHelper::ScreenHeight)/2.f;
		}
	}
}

#if SAC_INGAME_EDITORS
void ParatrooperSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    ParatrooperComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
