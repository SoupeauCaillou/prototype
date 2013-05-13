#include "DCASystem.h"

#include "base/TouchInputManager.h"

#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/norm.hpp"

INSTANCE_IMPL(DCASystem);

DCASystem::DCASystem() : ComponentSystemImpl <DCAComponent>("DCA") {
    DCAComponent dc;
    componentSerializer.add(new Property<float>("maximal_distance_for_activation", OFFSET(maximalDistanceForActivation, dc), 0.001));
    componentSerializer.add(new Property<float>("fire_rate", OFFSET(fireRate.value, dc), 0.001));
    componentSerializer.add(new Property<float>("puissance", OFFSET(puissance, dc), 0.001));
    componentSerializer.add(new Property<float>("dispersion", OFFSET(dispersion, dc), 0.001));
    componentSerializer.add(new Property<int>("fire_mode", OFFSET(fireMode, dc), 0));
    componentSerializer.add(new Property<float>("burst_rest_time", OFFSET(burstRestTime, dc), 0.001));
    componentSerializer.add(new EntityProperty("turret", OFFSET(turret, dc)));

}

void DCASystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(DCA, e, dc)
        dc->fireRate.accum += dt * dc->fireRate.value;

        if (!dc->shoot) {
            dc->fireRate.accum = glm::min(1.0f, dc->fireRate.accum);
            dc->burstBulletCount = 0;
            continue;
        }

        glm::vec2 fireDirection = dc->targetPoint - TRANSFORM(e)->position;

        //too far away! skip it
        if ( glm::length2(fireDirection) > dc->maximalDistanceForActivation * dc->maximalDistanceForActivation) {
            LOGW_EVERY_N(180, "can't shoot! too far away dude");
            continue;
        }
        fireDirection = glm::normalize(fireDirection);


        while (dc->fireRate.accum > 1.f) {
            Entity bullet = theEntityManager.CreateEntity("bullet",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("bullet"));

            //bullet comes from the DCA
            TRANSFORM(bullet)->position = TRANSFORM(e)->position;

            //add a random dispersion
            float randAngleDispersion = glm::gaussRand(0.f, dc->dispersion / 3.f);
            glm::vec2 randDispersedDirection = glm::rotate(fireDirection, randAngleDispersion);
            PHYSICS(bullet)->addForce(glm::vec2(0.f), dc->puissance * randDispersedDirection, 1/60.);


            --dc->fireRate.accum;
            if (dc->fireMode == DCAComponent::EFireMode::BURST) {
                ++dc->burstBulletCount;

                if (dc->burstBulletCount == 3) {
                    dc->fireRate.accum = - dc->burstRestTime;
                    dc->burstBulletCount = 0;
                }
            }
        }
        dc->shoot = false;
	}
}

#if SAC_INGAME_EDITORS
void DCASystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    DCAComponent* dc = Get(entity, false);
    if (!dc) return;

    TwAddVarRW(bar, "targetPoint x", TW_TYPE_FLOAT, &dc->targetPoint.x, "group=DCA precision=2 step=0,01");
    TwAddVarRW(bar, "targetPoint y", TW_TYPE_FLOAT, &dc->targetPoint.y, "group=DCA precision=2 step=0,01");
    TwAddVarRW(bar, "fire_rate", TW_TYPE_FLOAT, &dc->fireRate.value, "group=DCA precision=2 step=0,01");
    TwAddVarRW(bar, "dispersion", TW_TYPE_FLOAT, &dc->dispersion, "group=DCA precision=2 step=0,01");

}
#endif
