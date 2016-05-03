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
#include "systems/ZSQDSystem.h"
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(PlayerSystem);

const glm::vec2 DIR[] = {
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, -1.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(-1.0f, 0.0f)
};

PlayerSystem::PlayerSystem() : ComponentSystemImpl<PlayerComponent>(HASH("Player", 0x75a3a9db)) {
}

void PlayerSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Player, entity, pc)
        glm::vec2 facing(0.0f);
        for (int i=0; i<4; i++) {
            if (pc->input.directions.primary[i] == InputState::Pressed) {
                ZSQD(entity)->directions.push_back(DIR[i]);
            }
            if (pc->input.directions.secondary[i] == InputState::Pressed) {
                facing += DIR[i];
            }
        }
        float l = glm::length(facing);
        if (l > 0) {
            pc->facingDirection = facing / l;
        } else {
            pc->facingDirection = facing;
        }

    END_FOR_EACH()
}
