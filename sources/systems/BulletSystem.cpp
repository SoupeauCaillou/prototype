#include "BulletSystem.h"

#include "systems/TransformationSystem.h"
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
        FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, pc)
            //kill the guy
            if (IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(para))) {
                pc->dead = true;
            }
        }

        FOR_EACH_ENTITY(Parachute, para)
            //destroy the parachute
            if (IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(para))) {
                theParachuteSystem.destroyParachute(para);
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
