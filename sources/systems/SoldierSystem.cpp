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
    componentSerializer.add(new Property<float>("health", OFFSET(health, tc)));
    componentSerializer.add(new Property<float>("max_speed_collision", OFFSET(maxSpeedCollision, tc)));
    componentSerializer.add(new Property<float>("braking_force", OFFSET(brakingForce, tc)));
    componentSerializer.add(new Property<int>("status", OFFSET(status, tc)));
    componentSerializer.add(new Property<float>("cooldown", OFFSET(cd.duration, tc)));
}

void SoldierSystem::DoUpdate(float dt) {
    std::map<PhysicsComponent*, glm::vec2> velocityFixes;

    for (auto p: components) {
        Entity e = p.first;
        auto* sc = p.second;

        // check for collision
        auto* pc = PHYSICS(e);
        float linearVel = glm::length(pc->linearVelocity);

        if (linearVel <= 0.1) {
            pc->linearVelocity = glm::vec2(0.0f);
            linearVel = 0.0f;
        } else {
            FLICK(e)->enabled = false;
            pc->addForce(glm::normalize(pc->linearVelocity) * sc->brakingForce, glm::vec2(0.0f), dt);
            TRANSFORM(e)->rotation = glm::atan2(pc->linearVelocity.y, pc->linearVelocity.x) - glm::pi<float>() * 0.5;
        }

        auto* cc = COLLISION(e);
        if (cc->collidedWithLastFrame) {
            auto* pc2 = PHYSICS(cc->collidedWithLastFrame);
            glm::vec2 direct = TRANSFORM(cc->collidedWithLastFrame)->position - TRANSFORM(e)->position;
            direct /= (glm::length(direct) + 0.0001);
            const glm::vec2 ortho (direct.y, -direct.x);

            // simple bump
            glm::vec2 vel = pc->linearVelocity * 0.5f;
            float ratio = (pc2->mass > 0) ? (pc2->mass / (pc->mass + pc2->mass)) : 1.0f;
            glm::vec2 newVelocity1 = direct * (- ratio * glm::dot(vel, direct)) + ortho * (ratio * glm::dot(vel, ortho));
            velocityFixes[pc] += newVelocity1;

            velocityFixes[pc2] += vel * (1.0f - ratio);

            if (sc->status == Status::Moving) {
                if (glm::length(pc->linearVelocity) + glm::length(pc2->linearVelocity) > 1) {
                    sc->status = Status::Bouncing;
                }
            }
        }

        if (sc->health <= 0) {
            FLICK(e)->enabled = false;
            RENDERING(e)->color = Color(1, 0, 0);
            PHYSICS(e)->mass = 2;
        }

        switch (sc->status) {
            case Status::CanAttack: {
                FLICK(e)->enabled = false;
                sc->status = Status::Attack;
                // attacks are handled in subsystem (knight, archer, ...)
                break;
            }
            case Status::Attack: {
                FLICK(e)->enabled = false;
                sc->status = Status::CoolDown;
                break;
            }
            case Status::Idle: {
                FLICK(e)->enabled = (sc->health > 0);
                if (FLICK(e)->status == FlickStatus::Moving) {
                    sc->status = Status::Moving;
                }
                break;
            }
            case Status::Bouncing: {
                FLICK(e)->enabled = false;
                if (linearVel <= 0) {
                    sc->status = Status::CoolDown;
                }
                break;
            }
            case Status::Moving: {
                FLICK(e)->enabled = false;
                if (linearVel <= 0) {
                    sc->flickingDistance = glm::distance(TRANSFORM(e)->position, FLICK(e)->flickingStartedAt);
                    sc->status = Status::CanAttack;
                }
                break;
            }
            case Status::CoolDown: {
                FLICK(e)->enabled = false;
                sc->cd.accum += dt;
                if (sc->cd.accum >= sc->cd.duration) {
                    sc->cd.accum = 0;
                    sc->status = Status::Idle;
                }
                break;
            }
        }
    }

    for (auto f: velocityFixes) {
        f.first->linearVelocity = f.second;
    }
}

