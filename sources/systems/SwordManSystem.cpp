#include "SwordManSystem.h"


INSTANCE_IMPL(SwordManSystem);

SwordManSystem::SwordManSystem() : ComponentSystemImpl<SwordManComponent>("SwordMan") {
    SwordManComponent tc;
    componentSerializer.add(new EntityProperty("left_hand", OFFSET(hands[0], tc)));
    componentSerializer.add(new EntityProperty("right_hand", OFFSET(hands[1], tc)));
}

void SwordManSystem::DoUpdate(float ) {
}

#if SAC_INGAME_EDITORS
void SwordManSystem::addEntityPropertiesToBar(Entity , TwBar* ) {

}
#endif

