#include "BulletSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/ParatrooperSystem.h"

#include "util/IntersectionUtil.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl <BulletComponent>("Bullet") {
    //BulletComponent dc;
}

void BulletSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Bullet, e, bc)
        FOR_EACH_ENTITY(Paratrooper, para)
            if (IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(para)))
                LOGI("collision!");
        }
	}
}

#if SAC_INGAME_EDITORS
void BulletSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    BulletComponent* dc = Get(entity, false);
    if (!dc) return;
}
#endif
