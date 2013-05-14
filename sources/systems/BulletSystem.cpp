#include "BulletSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/ParachuteSystem.h"

#include "util/IntersectionUtil.h"
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl <BulletComponent>("Bullet") {
    //BulletComponent dc;
}

void BulletSystem::DoUpdate(float) {
//    FOR_EACH_ENTITY_COMPONENT(Bullet, e, bc)
    FOR_EACH_ENTITY(Bullet, e)

        auto tc = TRANSFORM(e);

        bool deleted = false;

        FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, pc)
            //don't shot landed guys
            if (PARATROOPER(para)->landed)
                continue;

            //kill the guy
            if (IntersectionUtil::rectangleRectangle(tc, TRANSFORM(para))) {
                pc->dead = true;
                deleted = true;
                theEntityManager.DeleteEntity(e);
                break;
            }
        }
        if (deleted)
            continue;

        FOR_EACH_ENTITY_COMPONENT(Parachute, parachute, pc)
            const auto transf = TRANSFORM(parachute);
            if (IntersectionUtil::rectangleRectangle(tc, transf)) {

                glm::vec2 pos = glm::rotate(tc->worldPosition - transf->worldPosition,
                    - transf->worldRotation);
                pc->damages.push_back(pos);

                Entity hole = theEntityManager.CreateEntity("hole", EntityType::Volatile,
                    theEntityManager.entityTemplateLibrary.load("hole"));
                PARACHUTE(parachute)->holes.push_back(hole);
                TRANSFORM(hole)->parent = parachute;
                TRANSFORM(hole)->position = pos;

                theEntityManager.DeleteEntity(e);
                break;
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
