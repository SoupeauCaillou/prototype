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
#include "systems/TextSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"

INSTANCE_IMPL(SoldierSystem);

SoldierSystem::SoldierSystem() : ComponentSystemImpl<SoldierComponent>("Soldier") {
	SoldierComponent sc;
    componentSerializer.add(new EntityProperty("ap_indicator", OFFSET(apIndicator, sc)));
    componentSerializer.add(new Property<int>("move_range", OFFSET(moveRange, sc), 0));
	componentSerializer.add(new IntervalProperty<int>("attack_range", OFFSET(attackRange, sc)));
	componentSerializer.add(new Property<int>("attack_damage", OFFSET(attackDamage, sc), 0));
	componentSerializer.add(new Property<int>("p_lance", OFFSET(pLance, sc), 0));
	componentSerializer.add(new Property<int>("defense_point", OFFSET(defensePoint, sc), 0));
    componentSerializer.add(new Property<int>("max_action_points_per_turn", OFFSET(maxActionPointsPerTurn, sc), 0));
    componentSerializer.add(new Property<int>("action_points_left", OFFSET(actionPointsLeft, sc), 0));
}

void SoldierSystem::DoUpdate(float) {

}

void SoldierSystem::UpdateUI(Entity e, SoldierComponent* sc) {
    if (sc->maxActionPointsPerTurn <= 0) {
        TEXT(sc->apIndicator)->show = false;
    } else {
        std::stringstream ss;
        ss << sc->actionPointsLeft << ' ' << sc->maxActionPointsPerTurn;
        TEXT(sc->apIndicator)->text = ss.str();
        ANCHOR(sc->apIndicator)->rotation = -TRANSFORM(e)->rotation;
    }
}
