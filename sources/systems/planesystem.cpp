#include "planesystem.h"

#include "systems/PhysicsSystem.h"

INSTANCE_IMPL(PlaneSystem);

PlaneSystem::PlaneSystem() : ComponentSystemImpl <PlaneComponent>("Plane") {

}

void PlaneSystem::DoUpdate(float dt) {
	FOR_EACH_ENTITY_COMPONENT(Plane, e, pc)
		PHYSICS(e)->linearVelocity = pc->speed;
	}
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