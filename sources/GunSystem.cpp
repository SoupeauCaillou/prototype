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
#include "GunSystem.h"
#include "PlayerSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/PhysicsSystem.h"
#include "base/EntityManager.h"
#include "util/SerializerProperty.h"
#include <glm/gtx/norm.hpp>

#include "HealthSystem.h"

#include "systems/ADSRSystem.h"
#include <sac/tweak.h>

INSTANCE_IMPL(GunSystem);

GunSystem::GunSystem() : ComponentSystemImpl<GunComponent>(HASH("Gun", 0x4d8ab4a8)) {

}

#if 0
static bool conditionToEnterState(GunState::Enum state, Entity player, Entity Gun) {
    switch (state) {
        case GunState::Resting: return true;
        case GunState::Defense: return glm::length2(PLAYER(player)->facingDirection) > 0;
        case GunState::Attack: return PLAYER(player)->input.actions[0] == InputState::Pressed && !ADSR(Gun)->active;
        default:
            return false;
    }
}
#endif

static GunState::Enum updateAttackState(GunComponent* sw, Entity gun, Entity player) {
    TWEAK(float, bulletSpeed) = 25;

    Entity bullet = theEntityManager.CreateEntityFromTemplate("bullet");
    TRANSFORM(bullet)->position = TRANSFORM(gun)->position;
    TRANSFORM(bullet)->rotation = TRANSFORM(gun)->rotation;

    PHYSICS(bullet)->linearVelocity = glm::rotate(
        glm::vec2(bulletSpeed, 0.0f),
        TRANSFORM(gun)->rotation);

    return GunState::CoolDown;
}

void GunSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Gun, entity, sw)
        Entity player = ANCHOR(entity)->parent;
        if (player == 0) {
            continue;
        }

        sw->stateDuration += dt;
        GunState::Enum currentState = sw->state;

        switch (sw->state) {
            case GunState::Fire: {
                RENDERING(entity)->color = Color(0.9, 0.5, 0.5);
                sw->state = updateAttackState(sw, entity, player);
            } break;
            case GunState::CoolDown: {
                TWEAK(float, gunCoolDown) = 1;
                if (sw->stateDuration >= gunCoolDown) {
                    sw->state = GunState::Resting;
                }
            } break;
            case GunState::Resting: {
                RENDERING(entity)->color = Color(0.5, 0.5, 0.5);
                if (sw->trigger) {
                    sw->state = GunState::Fire;
                    sw->trigger = false;
                }
            } break;
        }

        if (sw->state != currentState) {
            sw->stateDuration = 0;
        }

#if 0
        const auto* cc = COLLISION(entity);
        for (int i=0; i<cc->collision.count; i++) {
            Entity with = cc->collision.with[i];
            // ignore self-collision
            if (with == player) {
                continue;
            }
            // hit someone -> instant-kill
            if (sw->state == GunState::Attack
                && ADSR(entity)->activationTime > ADSR(entity)->attackTiming) {
                if (COLLISION(with)->group == 1) {
                    HEALTH(with)->currentHP = 0;
                    HEALTH(with)->hitBy = entity;
                }
            }
        }
#endif
    END_FOR_EACH()
}
