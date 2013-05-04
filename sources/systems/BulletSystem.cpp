#include "BulletSystem.h"

#include "systems/ParticuleSystem.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl <BulletComponent>("Bullet") {
    BulletComponent dc;
}

void BulletSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Bullet, e, dc)
        LOGI_EVERY_N(100, components.size() << ": " <<  PARTICULE(e)->forceAmplitude.t1 << " and " << PARTICULE(e)->forceAmplitude.t2);
        break;
	}
}

#if SAC_INGAME_EDITORS
void BulletSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    BulletComponent* dc = Get(entity, false);
    if (!dc) return;
}
#endif
