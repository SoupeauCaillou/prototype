#include "DCASystem.h"

#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

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
        dc->fireRate.accum += dt;
        while (dc->fireRate.accum > 1.f / dc->fireRate.value) {
            Entity bullet = theEntityManager.CreateEntity("bullet",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("bullet"));

            TRANSFORM(bullet)->position = TRANSFORM(e)->position;
            PHYSICS(bullet)->addForce(dc->puissance * glm::normalize(dc->direction), glm::vec2(0, 0), dt);
            dc->fireRate.accum -= 1 / dc->fireRate.value;
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
