#include "ParatrooperSystem.h"

#include "systems/PlayerSystem.h"

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
			//touching the ground (landing)
			if (IntersectionUtil::pointRectangle(glm::vec2(TRANSFORM(e)->worldPosition.x,
				-PlacementHelper::ScreenHeight/2.f), TRANSFORM(e)->worldPosition, TRANSFORM(e)->size)) {
				RENDERING(e)->color = PLAYER(pc->owner)->playerColor;
				if (pc->dead || glm::abs(PHYSICS(e)->linearVelocity.y) > 3.f){
					LOGW("Soldier '" << theEntityManager.entityName(e) << e << "' crashed at speed " << glm::abs(PHYSICS(e)->linearVelocity.y));
					pc->dead = true;
					PHYSICS(e)->mass = 0;
					AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;
					AUTO_DESTROY(e)->params.lifetime.freq.value = 1;
				}
				else {
					LOGW("Soldier '" << theEntityManager.entityName(e) << e << "' landed");
					PHYSICS(e)->linearVelocity.y = 0;
				}
				PHYSICS(e)->gravity = glm::vec2(0.f);

				//delete the parachute if any
                Entity parent = TRANSFORM(e)->parent;
                if (parent) {
                    TRANSFORM(e)->z = TRANSFORM(parent)->z;
                    TRANSFORM(e)->position = TRANSFORM(e)->worldPosition;
                    TRANSFORM(e)->parent = 0;
                    theEntityManager.DeleteEntity(parent);
                }

				pc->landed = true;
			}
		}
		else {
            FOR_EACH_ENTITY(Player, p)
	            if (pc->owner == p)
	                continue;
	            if (!pc->dead && IntersectionUtil::pointRectangle(TRANSFORM(e)->position, TRANSFORM(p)->position, TRANSFORM(p)->size))
	                ++PLAYER(pc->owner)->score;
	            AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;
	            AUTO_DESTROY(e)->params.lifetime.freq.value = 0;
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
