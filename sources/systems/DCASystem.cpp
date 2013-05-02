#include "DCASystem.h"

#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"

INSTANCE_IMPL(DCASystem);

DCASystem::DCASystem() : ComponentSystemImpl <DCAComponent>("DCA") {
    DCAComponent dc;
    componentSerializer.add(new Property<glm::vec2>("direction", OFFSET(direction, dc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("fire_rate", OFFSET(fireRate.value, dc), 0.001));
    componentSerializer.add(new Property<float>("puissance", OFFSET(puissance, dc), 0.001));
    componentSerializer.add(new Property<float>("dispersion", OFFSET(dispersion, dc), 0.001));
}

void DCASystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(DCA, e, dc)
        dc->direction = glm::normalize(dc->direction);
        dc->fireRate.accum += dt * dc->fireRate.value;
        while (dc->fireRate.accum > 1.f) {
            Entity bullet = theEntityManager.CreateEntity("bullet",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("bullet"));

            TRANSFORM(bullet)->position = TRANSFORM(e)->position;

            //add a random dispersion
            float randAngleDispersion = glm::gaussRand(0.f, dc->dispersion);
            glm::vec2 randDispersedDirection = glm::rotate(dc->direction, randAngleDispersion);
            PHYSICS(bullet)->addForce(dc->puissance * randDispersedDirection, glm::vec2(0, 0), dt);
            --dc->fireRate.accum;
        }
	}
}

#if SAC_INGAME_EDITORS
void DCASystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    DCAComponent* dc = Get(entity, false);
    if (!dc) return;

    TwAddVarRW(bar, "direction x", TW_TYPE_FLOAT, &dc->direction.x, "group=DCA precision=2 step=0,01");
    TwAddVarRW(bar, "direction y", TW_TYPE_FLOAT, &dc->direction.y, "group=DCA precision=2 step=0,01");
    TwAddVarRW(bar, "fire_rate", TW_TYPE_FLOAT, &dc->fireRate.value, "group=DCA precision=2 step=0,01");
    TwAddVarRW(bar, "dispersion", TW_TYPE_FLOAT, &dc->dispersion, "group=DCA precision=2 step=0,01");

}
#endif
