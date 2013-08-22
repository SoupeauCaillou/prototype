#include "OrcSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "base/Interval.h"

INSTANCE_IMPL(OrcSystem);

OrcSystem::OrcSystem() : ComponentSystemImpl<OrcComponent>("Orc") {
    OrcComponent ac;
    componentSerializer.add(new EntityProperty("weapon", OFFSET(weapon, ac)));
	componentSerializer.add(new Property<float>("level", OFFSET(level, ac)));
	componentSerializer.add(new Property<float>("max_speed", OFFSET(maxSpeed, ac)));
	componentSerializer.add(new Property<float>("attack_speed", OFFSET(attackSpeed, ac)));
	componentSerializer.add(new Property<float>("speed", OFFSET(speed, ac)));

}

void OrcSystem::DoUpdate(float dt) {
	for (auto& p: components) {
		const Entity orc = p.first;
		auto* ac = p.second;

	}
}
