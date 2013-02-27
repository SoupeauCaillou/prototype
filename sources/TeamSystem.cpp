#include "TeamSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"
#include "BallSystem.h"
#include "FieldPlayerSystem.h"

INSTANCE_IMPL(TeamSystem);

TeamSystem::TeamSystem() : ComponentSystemImpl<TeamComponent>("Team") {
    /* nothing saved */
}

void TeamSystem::DoUpdate(float dt) {
    const Entity ball = theBallSystem.RetrieveAllEntityWithComponent()[0];

    FOR_EACH_ENTITY_COMPONENT(Team, e, comp)
        
    }
}

#ifdef INGAME_EDITORS
void TeamSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    TeamComponent* tc = Get(entity, false);
    if (!tc) return;

}
#endif
