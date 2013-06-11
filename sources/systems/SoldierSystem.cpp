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
#include "SoldierSystem.h"

INSTANCE_IMPL(SoldierSystem);

SoldierSystem::SoldierSystem() : ComponentSystemImpl<SoldierComponent>("Soldier") {
	SoldierComponent sc;
    componentSerializer.add(new Property<int>("move_range", OFFSET(moveRange, sc), 0));
	componentSerializer.add(new Property<int>("vision_range", OFFSET(visionRange, sc), 0));
	componentSerializer.add(new IntervalProperty<unsigned>("attack_range", OFFSET(attackRange, sc)));
	componentSerializer.add(new Property<int>("attack_damage", OFFSET(attackDamage, sc), 0));
	componentSerializer.add(new Property<int>("p_lance", OFFSET(pLance, sc), 0));
	componentSerializer.add(new Property<int>("defense_point", OFFSET(defensePoint, sc), 0));
    componentSerializer.add(new Property<int>("max_action_points_per_turn", OFFSET(maxActionPointsPerTurn, sc), 0));
    componentSerializer.add(new Property<int>("action_points_left", OFFSET(actionPointsLeft, sc), 0));
}

void SoldierSystem::DoUpdate(float) {

}
