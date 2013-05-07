#include "PlaneSystem.h"

#include "systems/ParatrooperSystem.h"

#include "base/PlacementHelper.h"

#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"

#include "util/IntersectionUtil.h"

INSTANCE_IMPL(PlaneSystem);

PlaneSystem::PlaneSystem() : ComponentSystemImpl <PlaneComponent>("Plane") {
	PlaneComponent pc;
    componentSerializer.add(new Property<glm::vec2>("speed", OFFSET(speed, pc), glm::vec2(0.001f, 0.f)));
    componentSerializer.add(new Property<int>("paratrooperLimit", OFFSET(paratrooperLimit, pc), 1));
    componentSerializer.add(new Property<int>("paratrooperAvailable", OFFSET(paratrooperAvailable, pc), 1));
    componentSerializer.add(new Property<float>("timeBetweenJumps", OFFSET(timeBetweenJumps.value, pc), 0.001f));
}

void PlaneSystem::DoUpdate(float dt) {
	FOR_EACH_ENTITY_COMPONENT(Plane, e, pc)
		PHYSICS(e)->linearVelocity = pc->speed;

		TransformationComponent *tc = TRANSFORM(e);
		if (!IntersectionUtil::rectangleRectangle(tc->worldPosition, tc->size, tc->worldRotation,
        	glm::vec2(0.f), glm::vec2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight), 0)) {
			tc->position.x = -tc->position.x;
			pc->paratrooperAvailable = pc->paratrooperLimit;
		}

		if (pc->dropOne) {
			if (pc->paratrooperAvailable && pc->timeBetweenJumps.accum > 1.f) {
				Entity paratrooper = theEntityManager.CreateEntity("paratrooper",
	                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("paratrooper"));
		        TRANSFORM(paratrooper)->position = TRANSFORM(e)->position;
		        PHYSICS(paratrooper)->linearVelocity.x = PHYSICS(e)->linearVelocity.x;
		        --pc->paratrooperAvailable;
			}
			pc->dropOne = false;
		}

		if (pc->timeBetweenJumps.accum > 1.f) {
			-- pc->timeBetweenJumps.accum;
		}

		pc->timeBetweenJumps.accum += dt * pc->timeBetweenJumps.value;
	}

}

#if SAC_INGAME_EDITORS
void PlaneSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    PlaneComponent* pc = Get(entity, false);
    if (!pc) return;
    TwAddVarRW(bar, "speed.X", TW_TYPE_FLOAT, &pc->speed.x, "group=Plane precision=2 step=0,01");
    TwAddVarRW(bar, "speed.Y", TW_TYPE_FLOAT, &pc->speed.y, "group=Plane precision=2 step=0,01");
    TwAddVarRW(bar, "Paratrooper", TW_TYPE_FLOAT, &pc->paratrooperLimit, "group=Plane precision=1");
    TwAddVarRW(bar, "Paratrooper", TW_TYPE_FLOAT, &pc->paratrooperAvailable, "group=Plane precision=1");
    TwAddVarRW(bar, "timeBetweenJumps", TW_TYPE_FLOAT, &pc->timeBetweenJumps.value, "group=Plane");
}
#endif
