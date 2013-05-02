#include "planesystem.h"

#include "util/IntersectionUtil.h"

#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"

INSTANCE_IMPL(PlaneSystem);

PlaneSystem::PlaneSystem() : ComponentSystemImpl <PlaneComponent>("Plane") {
	PlaneComponent pc;
    componentSerializer.add(new Property<glm::vec2>("speed", OFFSET(speed, pc), glm::vec2(0.001f, 0.f)));
    componentSerializer.add(new Property<int>("paratrooper", OFFSET(paratrooper, pc), 1));
    componentSerializer.add(new Property<int>("timeBetweenJumps", OFFSET(timeBetweenJumps, pc), 0.001f));

    worldSize = glm::vec2(0.f);
}

void PlaneSystem::DoUpdate(float dt) {
	FOR_EACH_ENTITY_COMPONENT(Plane, e, pc)
		PHYSICS(e)->linearVelocity = pc->speed;

		if (worldSize != glm::vec2(0.f)) {
			TransformationComponent *tc = TRANSFORM(e);
			if (!IntersectionUtil::rectangleRectangle(tc->worldPosition, tc->size, tc->worldRotation,
            	glm::vec2(0.f), worldSize, 0)) {
				tc->position.x = -tc->position.x;
			}
		}
	}
}

void PlaneSystem::setWorldSize(const glm::vec2 ws) {
	worldSize = ws;
}

#if SAC_INGAME_EDITORS
void PlaneSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    PlaneComponent* pc = Get(entity, false);
    if (!pc) return;
    TwAddVarRW(bar, "speed.X", TW_TYPE_FLOAT, &pc->speed.x, "group=Plane precision=2 step=0,01");
    TwAddVarRW(bar, "speed.Y", TW_TYPE_FLOAT, &pc->speed.y, "group=Plane precision=2 step=0,01");
    TwAddVarRW(bar, "Paratrooper", TW_TYPE_FLOAT, &pc->paratrooper, "group=Plane precision=1");
    TwAddVarRW(bar, "timeBetweenJumps", TW_TYPE_FLOAT, &pc->timeBetweenJumps, "group=Plane");
}
#endif