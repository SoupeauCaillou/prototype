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
#include "PlayerSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/ZSQDSystem.h"
#include <glm/gtx/compatibility.hpp>

#include "HealthSystem.h"
#include "EquipmentSystem.h"

INSTANCE_IMPL(PlayerSystem);

PlayerSystem::PlayerSystem() : ComponentSystemImpl<PlayerComponent>(HASH("Player", 0x75a3a9db)) {
}

void PlayerSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Player, entity, pc)
        if (glm::length(pc->input.directions.primary)) {
            ZSQD(entity)->directions.push_back(pc->input.directions.primary);
        }
        glm::vec2 facing = pc->input.directions.secondary;
        float l = glm::length(facing);
        if (l > 0) {
            pc->facingDirection = facing / l;
        } else {
            pc->facingDirection = facing;
        }

        /* hits */
        const auto* cc = COLLISION(entity);
        for (int i=0; i<cc->collision.count; i++) {
            Entity with = cc->collision.with[i];
            if (with == 0) {
                continue;
            }
            auto* ccWith = COLLISION(with);

            /* attack sword hit */
            if (ccWith->group == 2) {
                /* ignore self-hits */
                if (with == EQUIPMENT(entity)->hands[0] ||
                    with == EQUIPMENT(entity)->hands[1]) {
                    continue;
                }
                HEALTH(entity)->currentHP = 0;
                HEALTH(entity)->hitBy = with;
            }
            /* bullet hit */
            if (ccWith->group == 8) {
                HEALTH(entity)->currentHP = 0;
                HEALTH(entity)->hitBy = with;
            }
        }


    END_FOR_EACH()
}
