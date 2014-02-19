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
#include "systems/TransformationSystem.h"

#include "systems/FlickSystem.h"

INSTANCE_IMPL(SoldierSystem);

SoldierSystem::SoldierSystem() : ComponentSystemImpl<SoldierComponent>("Soldier") {
    SoldierComponent tc;
    componentSerializer.add(new EntityProperty("player", OFFSET(player, tc)));
    componentSerializer.add(new Property<bool>("health", OFFSET(health, tc)));
    componentSerializer.add(new Property<float>("max_speed_collision", OFFSET(maxSpeedCollision, tc)));
}

void SoldierSystem::DoUpdate(float) {
    std::map<PhysicsComponent*, glm::vec2> velocityFixes;

    for (auto p: components) {
        Entity e = p.first;
        auto* sc = p.second;

        // check for collision
        if (sc->health > 0) {
            auto* pc = PHYSICS(e);
            float linearVel = glm::length(pc->linearVelocity);

            if (linearVel <= 0.1) {
                FLICK(e)->enabled = true;
                pc->linearVelocity = glm::vec2(0.0f);
                linearVel = 0.0f;
            } else {
                FLICK(e)->enabled = false;
            }

            auto* cc = COLLISION(e);
            if (cc->collidedWithLastFrame) {
                auto* pc2 = PHYSICS(cc->collidedWithLastFrame);
                glm::vec2 direct = TRANSFORM(cc->collidedWithLastFrame)->position - TRANSFORM(e)->position;
                direct /= (glm::length(direct) + 0.0001);
                const glm::vec2 ortho (direct.y, -direct.x);

                // simple bump
                glm::vec2 vel = pc->linearVelocity * 0.5f;
                float ratio = pc2->mass / (pc->mass + pc2->mass);
                glm::vec2 newVelocity1 = direct * (- ratio * glm::dot(vel, direct)) + ortho * (ratio * glm::dot(vel, ortho));
                velocityFixes[pc] += newVelocity1;

                velocityFixes[pc2] += vel * (1.0f - ratio);
            }
        }

        if (sc->health <= 0) {
            FLICK(e)->enabled = false;
            RENDERING(e)->color = Color(1, 0, 0);
            PHYSICS(e)->mass = 0;
        }
    }

    for (auto f: velocityFixes) {
        f.first->linearVelocity = f.second;
    }
}

