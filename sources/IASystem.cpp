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
#include "IASystem.h"
#include "util/SerializerProperty.h"
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"
#include "PrototypeGame.h"
#include "util/Random.h"
#include "util/Misc.h"
#include "util/IntersectionUtil.h"
#include "EquipmentSystem.h"
#include "GunSystem.h"
#include "util/Draw.h"

#include <glm/gtx/norm.hpp>

INSTANCE_IMPL(IASystem);

struct IAStateHandler {
    virtual void onEnter(PrototypeGame* game, Entity entity, IAComponent* ia, IAState::Enum from) {
        ia->stateDuration = 0;
    }

    virtual IAState::Enum update(PrototypeGame* game, Entity entity, IAComponent* ia, float dt) = 0;

    virtual void onExit(PrototypeGame* game, Entity entity, IAComponent* ia, IAState::Enum to) {}
};

struct IdleState : public IAStateHandler{
    void onEnter(PrototypeGame* game, Entity entity, IAComponent* ia, IAState::Enum from) override {
        IAStateHandler::onEnter(game, entity, ia, from);
        TWEAK(float, minIdle) = 0.5;
        TWEAK(float, maxIdle) = 1.5;
        ia->idleDuration = Random::Float(minIdle, maxIdle);
    }

    IAState::Enum update(PrototypeGame* game, Entity entity, IAComponent* ia, float dt) override {
        return IAState::Moving;

        Draw::Point(TRANSFORM(entity)->position, Color(1, 1, 1));
        if (ia->stateDuration <= ia->idleDuration) {
            return IAState::Idle;
        }

        for (int i=0; i<2; i++) {
            if (EQUIPMENT(entity)->hands[i] &&
                GUN(EQUIPMENT(entity)->hands[i])->state == GunState::Resting) {
                return IAState::Aiming;
            }
        }

        return IAState::Moving;
    }
};

static bool hitAFriend(const glm::vec2& from, const glm::vec2& dir, Entity me) {
    // fix ray-cast instead
    const auto& friends = theIASystem.RetrieveAllEntityWithComponent();
    for (auto f: friends) {
        if (f == me) continue;

        if (IntersectionUtil::lineRectangle(
            from, dir * 100.0f,
            TRANSFORM(f)->position, TRANSFORM(f)->size, TRANSFORM(f)->rotation,
            nullptr, nullptr)) {
            return true;
        }
    }
    return false;
}

struct AimingState : public IAStateHandler{
    IAState::Enum update(PrototypeGame* game, Entity entity, IAComponent* ia, float dt) override {
        // Draw::Point(TRANSFORM(entity)->position, Color(1, 1, 0));

        {
            float dR = aim(
                TRANSFORM(entity)->position,
                TRANSFORM(entity)->rotation,
                TRANSFORM(game->guy[0])->position);
            TWEAK(float, rotationSpeed) = 2.0f;
            float dr = glm::min(dR, glm::sign(dR) * rotationSpeed * dt);
            TRANSFORM(entity)->rotation += dr;
        }

        for (int i=0; i<2; i++) {
            Entity g = EQUIPMENT(entity)->hands[i];
            if (g) {
                glm::vec2 diff2 = TRANSFORM(game->guy[0])->position - TRANSFORM(g)->position;
                TWEAK(float, gunAngleCosBoundaries) = 0.8;
                float a = atan2(diff2.y, diff2.x) - TRANSFORM(entity)->rotation;

                if (gunAngleCosBoundaries <= glm::cos(a) &&
                    GUN(g)->state == GunState::Resting) {
                    ANCHOR(g)->rotation = a;

                    TWEAK(float, aimingDuration) = 1.0f;
                    if (aimingDuration <= ia->stateDuration) {
                        /* check line of sight */
                        if (hitAFriend(TRANSFORM(g)->position, diff2, entity)) {
                            return IAState::Idle;
                        }

                        GUN(g)->trigger = true;

                        if (Random::Int(0, 1)) {
                            return IAState::Aiming;
                        } else {
                            return IAState::Idle;
                        }
                    }
                }
            }
        }
        return IAState::Aiming;
    }
};

struct MovingState : public IAStateHandler{
    void onEnter(PrototypeGame* game, Entity entity, IAComponent* ia, IAState::Enum from) override {
        IAStateHandler::onEnter(game, entity, ia, from);

        // move towards nearest friend
        Entity nearest = 0;
        float best = FLT_MAX;

        const auto& friends = theIASystem.RetrieveAllEntityWithComponent();
        for (auto e: friends) {
            if (e == entity) continue;
            float d = glm::distance2(TRANSFORM(entity)->position, TRANSFORM(e)->position);
            if (d < best) {
                best = d;
                nearest = e;
            }
        }

        // move away from enemy
        glm::vec2 away = glm::normalize(
            TRANSFORM(entity)->position - TRANSFORM(game->guy[0])->position);

        if (nearest) {
            ia->moveDirection = away * 0.5f + glm::normalize(TRANSFORM(nearest)->position - TRANSFORM(entity)->position) * 0.5f;
        } else {
            ia->moveDirection = away;
        }

        TWEAK(float, minMoveDuration) = 1.2;
        TWEAK(float, maxMoveDuration) = 3.2;
        ia->moveDuration = Random::Float(minMoveDuration, maxMoveDuration);
    }

    IAState::Enum update(PrototypeGame* game, Entity entity, IAComponent* ia, float dt) override {
        // Draw::Point(TRANSFORM(entity)->position, Color(0, 0, 1));

        if (ia->moveDuration <= ia->stateDuration) {
            return IAState::Aiming;
        }

        float threshold = TRANSFORM(entity)->size.x * 2.5f;
        const auto& friends = theIASystem.RetrieveAllEntityWithComponent();
        for (auto f: friends) {
            if (f == entity) continue;
            glm::vec2 diff = TRANSFORM(f)->position - TRANSFORM(entity)->position;
            if (glm::dot(diff, ia->moveDirection) > 0) {
                float d = glm::length(diff);
                if (d < threshold) {
                    glm::vec2 normal = glm::vec2(-diff.y, diff.x) / d;
                    ia->moveDirection = glm::normalize(
                        ia->moveDirection +
                        normal * (1.0f - d / threshold));
                }
            }
        }

        {
            float absLimitX = 0.5f * TRANSFORM(game->battleground)->size.x;
            float absLimitY = 0.5f * TRANSFORM(game->battleground)->size.y;
            glm::vec2 futurePosition =
                TRANSFORM(entity)->position +
                ia->moveDirection *
                ZSQD(entity)->maxSpeed * dt;
            float size = TRANSFORM(entity)->size.x * 0.75;

            if (((futurePosition.x + size) > absLimitX) && ia->moveDirection.x > 0) {
                ia->moveDirection.x = - ia->moveDirection.x * Random::Float(0, 1);
            } else if (((futurePosition.x - size) < -absLimitX) && ia->moveDirection.x < 0) {
                ia->moveDirection.x = -ia->moveDirection.x * Random::Float(0, 1);
            }

            if (((futurePosition.y + size) > absLimitY) && ia->moveDirection.y > 0) {
                ia->moveDirection.y = -ia->moveDirection.y * Random::Float(0, 1);
            } else if (((futurePosition.y - size) < -absLimitY) && ia->moveDirection.y < 0) {
                ia->moveDirection.y = -ia->moveDirection.y * Random::Float(0, 1);
            }
        }

        ia->moveDirection = glm::normalize(ia->moveDirection);
        // Draw::Vec2(TRANSFORM(entity)->position, ia->moveDirection);
        ZSQD(entity)->directions.push_back(ia->moveDirection);
        return IAState::Moving;
    }
};

struct EnterArenaState : public IAStateHandler{
    IAState::Enum update(PrototypeGame* game, Entity entity, IAComponent* ia, float dt) override {
        float absLimitX = 0.5f * TRANSFORM(game->battleground)->size.x;
        float absLimitY = 0.5f * TRANSFORM(game->battleground)->size.y;

        const glm::vec2& pos = TRANSFORM(entity)->position;
        const glm::vec2& size = TRANSFORM(entity)->size;
        float diffX = glm::abs(pos.x) + size.x - absLimitX;
        float diffY = glm::abs(pos.y) + size.y - absLimitY;

        if (diffX < 0 && diffY < 0) {
            return IAState::Idle;
        }

        if (0 < diffX) {
            ZSQD(entity)->directions.push_back(glm::vec2(-glm::sign(pos.x), 0.0f));
        } else {
            ZSQD(entity)->directions.push_back(glm::vec2(0.0f, -glm::sign(pos.y)));
        }

        return IAState::EnterArena;
    }
};

static std::array<IAStateHandler*, IAState::Count> handlers;

IASystem::IASystem() : ComponentSystemImpl<IAComponent>(HASH("IA", 0x58109180)) {
    handlers[IAState::Idle] = new IdleState;
    handlers[IAState::Aiming] = new AimingState;
    handlers[IAState::Moving] = new MovingState;
    handlers[IAState::EnterArena] = new EnterArenaState;
}

void IASystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(IA, entity, ia)
        IAState::Enum newState = handlers[ia->state]->update(game, entity, ia, dt);
        if (newState != ia->state) {
            IAState::Enum previous = ia->state;
            handlers[ia->state]->onExit(game, entity, ia, newState);
            ia->state = newState;
            handlers[ia->state]->onEnter(game, entity, ia, previous);
        }
        ia->stateDuration += dt;
    END_FOR_EACH()
}
