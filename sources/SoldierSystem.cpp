/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "SoldierSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"

#include "WeaponSystem.h"

INSTANCE_IMPL(SoldierSystem);

SoldierSystem::SoldierSystem() : ComponentSystemImpl<SoldierComponent>("Soldier") {
    SoldierComponent tc;
    componentSerializer.add(new EntityProperty("weapon", OFFSET(weapon, tc)));
    componentSerializer.add(new EntityProperty("team", OFFSET(team, tc)));
    componentSerializer.add(new Property<float>("health", OFFSET(health, tc), 0.001));
}

void SoldierSystem::DoUpdate(float dt) {
    for (auto p: components) {
        auto entity = p.first;
        auto* sc = p.second;

        std::stringstream ss;
        ss << (int)sc->health;
        TEXT(entity)->text = ss.str();

        if (sc->health == 0) {
            RENDERING(entity)->shape = Shape::Triangle;
            COLLISION(entity)->group = 0;
            BUTTON(entity)->enabled = false;
            WEAPON(sc->weapon)->fire = false;
        }
    }
}

