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
#include "SwordSystem.h"
#include "PlayerSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/BackInTimeSystem.h"
#include "systems/RenderingSystem.h"
#include "util/SerializerProperty.h"
#include <glm/gtx/norm.hpp>

#include "systems/ADSRSystem.h"
#include "systems/PhysicsSystem.h"
#include <sac/tweak.h>

INSTANCE_IMPL(SwordSystem);

SwordSystem::SwordSystem() : ComponentSystemImpl<SwordComponent>(HASH("Sword", 0x7a81df69)) {

}

static bool conditionToEnterState(SwordState::Enum state, Entity player, Entity sword) {
    switch (state) {
        case SwordState::Resting: return true;
        case SwordState::Defense: return glm::length2(PLAYER(player)->facingDirection) > 0;
        case SwordState::Attack: return PLAYER(player)->input.actions[0] == InputState::Pressed && !ADSR(sword)->active;
        default:
            return false;
    }
}

static SwordState::Enum updateAttackState(SwordComponent* sw, Entity sword, Entity player) {
    TWEAK(float, attackDuration) = 0.3;
    TWEAK(float, attackOffset) = 0.65;

    RENDERING(sword)->show = true;

    ADSR(sword)->active = true;

    float angle = ADSR(sword)->value - glm::pi<float>() * 0.5f;

    ANCHOR(sword)->position = glm::rotate(
        glm::vec2(attackOffset, 0.0f),
        angle);
    ANCHOR(sword)->rotation = angle;


    float dur = ADSR(sword)->activationTime;
    if (dur > attackDuration) {
        ADSR(sword)->active = false;
        return SwordState::Resting;
    } else {
        return SwordState::Attack;
    }
}

static SwordState::Enum updateDefenseState(SwordComponent* sw, Entity sword, Entity player) {
    TWEAK(float, defenseMaxDuration) = 5;
    TWEAK(float, defenseOffset) = 0.45;
    TWEAK(float, xOffset) = 0.2;
    TWEAK(float, yOffset) = -0.35;

    RENDERING(sword)->show = true;

    if (conditionToEnterState(SwordState::Attack, player, sword)) {
        return SwordState::Attack;
    }
    if (sw->stateDuration > defenseMaxDuration) {
        return SwordState::Resting;
    }
    if (!conditionToEnterState(SwordState::Defense, player, sword)) {
        return SwordState::Resting;
    }

    const glm::vec2& facingDirection = PLAYER(player)->facingDirection;

    float desiredAngle =
        atan2(facingDirection.y, facingDirection.x) +
        glm::pi<float>() * 0.5f;

    ANCHOR(sword)->position = glm::rotate(
        facingDirection * defenseOffset,
        -TRANSFORM(player)->rotation);
    ANCHOR(sword)->rotation = desiredAngle - TRANSFORM(player)->rotation;

    return SwordState::Defense;
}

void SwordSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Sword, entity, sw)
        Entity player = ANCHOR(entity)->parent;
        if (player == 0) {
            continue;
        }

        sw->stateDuration += dt;
        SwordState::Enum currentState = sw->state;

        switch (sw->state) {
            case SwordState::Attack: {
                sw->state = updateAttackState(sw, entity, player);
            } break;
            case SwordState::Defense: {
                LOGF_IF(ADSR(entity)->active, "Sword's ADSR should only be active in attack mode");
                sw->state = updateDefenseState(sw, entity, player);
            } break;
            case SwordState::Resting: {
                LOGF_IF(ADSR(entity)->active, "Sword's ADSR should only be active in attack mode");
                RENDERING(entity)->show = false;
                if (conditionToEnterState(SwordState::Attack, player, entity)) {
                    sw->stateDuration = 0;
                    sw->state = updateAttackState(sw, entity, player);
                }
                else if (conditionToEnterState(SwordState::Defense, player, entity)) {
                    sw->stateDuration = 0;
                    sw->state = updateDefenseState(sw, entity, player);
                }
            } break;
        }

        if (sw->state != currentState) {
            sw->stateDuration = 0;
        }

        TWEAK(int, swordCollisionGroup) = 2;
        /* enable collision when ready to hit */
        auto* cc = COLLISION(entity);
        if (sw->state == SwordState::Attack
                && ADSR(entity)->activationTime > ADSR(entity)->attackTiming) {
            cc->group = swordCollisionGroup;
        } else if (sw->state == SwordState::Defense) {
            cc->group = 4;
        } else {
            cc->group = 0;
        }

        for (int i=0; i<cc->collision.count; i++) {
            auto* cc2 = COLLISION(cc->collision.with[i]);
            if (cc2->group == 8) {
                Entity bullet = cc->collision.with[i];
                LOGI("Hit by a bullet!");

                /* treat sword as a line */
                glm::vec2 sword = glm::rotate(glm::vec2(1.0f, 0.0f), TRANSFORM(entity)->rotation);
                float d = glm::dot(PHYSICS(bullet)->linearVelocity, sword);
                PHYSICS(bullet)->linearVelocity = -PHYSICS(bullet)->linearVelocity + 2 * d * sword;
                TRANSFORM(bullet)->rotation = atan2(PHYSICS(bullet)->linearVelocity.y, PHYSICS(bullet)->linearVelocity.x);
                PHYSICS(bullet)->mass = 1;
                cc2->group = 0;
                LOGI(cc->collision.at[i]);
                TRANSFORM(bullet)->position =
                    glm::lerp(BACK_IN_TIME(bullet)->position,
                        TRANSFORM(bullet)->position,
                        cc->collision.at[i]);
            }
        }

    END_FOR_EACH()
}
