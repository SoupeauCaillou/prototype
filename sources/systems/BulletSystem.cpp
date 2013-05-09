#include "BulletSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/ParachuteSystem.h"

#include "util/IntersectionUtil.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl <BulletComponent>("Bullet") {
    //BulletComponent dc;
}

void BulletSystem::DoUpdate(float) {
//    FOR_EACH_ENTITY_COMPONENT(Bullet, e, bc)
    FOR_EACH_ENTITY(Bullet, e)

        auto tc = TRANSFORM(e);

        FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, pc)
            //kill the guy
            if (IntersectionUtil::rectangleRectangle(tc, TRANSFORM(para))) {
                pc->dead = true;
                theEntityManager.DeleteEntity(e);
            }
        }

        FOR_EACH_ENTITY_COMPONENT(Parachute, para, pc)
            if (IntersectionUtil::rectangleRectangle(tc, TRANSFORM(para))) {
                pc->damages.push_back(tc->worldPosition - TRANSFORM(para)->worldPosition + TRANSFORM(para)->size / 2.f);
                theEntityManager.DeleteEntity(e);
            }
        }
	}
}

#if SAC_INGAME_EDITORS
void BulletSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    BulletComponent* dc = Get(entity, false);
    if (!dc) return;
}
#endif
