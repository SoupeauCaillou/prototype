#include "ParatrooperSystem.h"

#include "systems/PlayerSystem.h"

#include "base/PlacementHelper.h"

#include "systems/AutoDestroySystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ParachuteSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

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
				if (pc->dead || glm::abs(PHYSICS(e)->linearVelocity.y) > 3.f) {
					LOGW("Soldier '" << theEntityManager.entityName(e) << e << "' crashed at speed " << glm::abs(PHYSICS(e)->linearVelocity.y));
					pc->dead = true;
					PHYSICS(e)->mass = 0;
					AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;
					AUTO_DESTROY(e)->params.lifetime.freq.value = 1;
				}
				else {
					LOGW("Soldier '" << theEntityManager.entityName(e) << e << "' landed");
					PHYSICS(e)->mass = 0;
                    ++PLAYER(pc->owner)->score;
				}
				PHYSICS(e)->gravity = glm::vec2(0.f);

				//delete the parachute if any
                if (pc->parachute) {
                    ParachuteSystem::DeleteParachute(pc->parachute);
                    pc->parachute = 0;
                }

				pc->landed = true;
			}
		}

		if (pc->dead) {
			PARTICULE(e)->emissionRate = 100;
		}
	}
}

#if SAC_INGAME_EDITORS
void ParatrooperSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    ParatrooperComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
