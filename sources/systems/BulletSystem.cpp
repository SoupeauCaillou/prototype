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
        FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, pc)
            //kill the guy
            if (IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(para))) {
                pc->dead = true;
            }
        }

        FOR_EACH_ENTITY_COMPONENT(Parachute, para, pc)
            glm::vec2 halfSize(TRANSFORM(para)->size);
            halfSize.x /= 2.f;

            if (IntersectionUtil::rectangleRectangle(
                TRANSFORM(e)->worldPosition, TRANSFORM(e)->size, TRANSFORM(e)->worldRotation,
                TRANSFORM(para)->worldPosition, halfSize, TRANSFORM(para)->worldRotation)) {
                RENDERING(para)->color.b += 0.5;
                pc->destroyedLeft = true;
            } else if (IntersectionUtil::rectangleRectangle(
                TRANSFORM(e)->worldPosition, TRANSFORM(e)->size, TRANSFORM(e)->worldRotation,
                TRANSFORM(para)->worldPosition + glm::vec2(halfSize.x, 0.f), halfSize, TRANSFORM(para)->worldRotation)) {
                RENDERING(para)->color.g += 0.5;
                RENDERING(para)->color.r += 0.5;
                pc->destroyedRight = true;
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
