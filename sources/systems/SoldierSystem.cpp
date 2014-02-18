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

#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/PhysicsSystem.h"

#include "systems/FlickSystem.h"

INSTANCE_IMPL(SoldierSystem);

SoldierSystem::SoldierSystem() : ComponentSystemImpl<SoldierComponent>("Soldier") {
    SoldierComponent tc;
    componentSerializer.add(new EntityProperty("player", OFFSET(player, tc)));
    componentSerializer.add(new Property<bool>("alive", OFFSET(alive, tc)));
    componentSerializer.add(new Property<float>("max_speed_collision", OFFSET(maxSpeedCollision, tc)));
}

void SoldierSystem::DoUpdate(float) {
    for (auto p: components) {
        Entity e = p.first;
        auto* sc = p.second;

        // check for collision
        if (sc->alive) {
            const float linearVel = glm::length(PHYSICS(e)->linearVelocity);

            FLICK(e)->enabled = (linearVel <= 0.01);

            const auto* cc = COLLISION(e);
            if (cc->collidedWithLastFrame) {
                if (linearVel > sc->maxSpeedCollision) {
                    sc->alive = false;
                }
            }

        }

        if (!sc->alive) {
            FLICK(e)->enabled = false;
            RENDERING(e)->color = Color(1, 0, 0);
            PHYSICS(e)->mass = 0;
        }
    }
}

