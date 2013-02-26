#include "BallSystem.h"
#include "systems/PhysicsSystem.h"

INSTANCE_IMPL(BallSystem);

BallSystem::BallSystem() : ComponentSystemImpl<BallComponent>("Ball") {
    /* nothing saved */
}

void BallSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Ball, e, comp)
        // add friction to ball
        if (PHYSICS(e)->linearVelocity.LengthSquared() > 0) {
            PHYSICS(e)->forces.push_back(std::make_pair(Force(PHYSICS(e)->linearVelocity * comp->friction, Vector2::Zero), dt));
        }
    }
}

#ifdef INGAME_EDITORS
void BallSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    BallComponent* tc = Get(entity, false);
    if (!tc) return;

}
#endif
