#include "UnitSystem.h"

#include "base/PlacementHelper.h"
#include "systems/TransformationSystem.h"

INSTANCE_IMPL(UnitSystem);

UnitSystem::UnitSystem() : ComponentSystemImpl<UnitComponent>(HASH("Unit", 0xb16d96c1)) {
    UnitComponent uc;
    componentSerializer.add(new Property<int>(HASH("index", 0x42bec51c), OFFSET(index, uc)));
}

void UnitSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Unit, e, uc)

    END_FOR_EACH()
}

