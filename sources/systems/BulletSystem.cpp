#include "BulletSystem.h"

#include "systems/ParatrooperSystem.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl <BulletComponent>("Bullet") {
    BulletComponent dc;
}

void BulletSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Bullet, e, bc)
        FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, pc)

        }
	}
}

#if SAC_INGAME_EDITORS
void BulletSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    BulletComponent* dc = Get(entity, false);
    if (!dc) return;
}
#endif
