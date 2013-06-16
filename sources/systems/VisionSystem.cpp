/*
    This file is part of Prototype.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "VisionSystem.h"
#include "PrototypeGame.h"
#include "systems/TransformationSystem.h"

INSTANCE_IMPL(VisionSystem);

VisionSystem::VisionSystem() : ComponentSystemImpl<VisionComponent>("Vision") {
	VisionComponent vc;
    componentSerializer.add(new Property<bool>("enabled", OFFSET(enabled, vc)));
	componentSerializer.add(new Property<int>("vision_range", OFFSET(visionRange, vc), 0));
}

void VisionSystem::DoUpdate(float) {
	for (auto it: components) {
		const Entity e = it.first;
		auto* vc = it.second;

		if (!vc->enabled)
			continue;

		const GridPos viewerPos = game->grid.positionToGridPos(TRANSFORM(e)->position);
		vc->visiblePositions = game->grid.viewRange(viewerPos, vc->visionRange);
	}
}
