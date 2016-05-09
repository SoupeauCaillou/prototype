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

#pragma once

#include "systems/System.h"

class PrototypeGame;

namespace InputState {
    enum Enum {
        Pressed, Released, None
    };
}

struct PlayerComponent {
    PlayerComponent() {
        facingDirection = glm::vec2(0.0f, 1.0f);
        input.directions.primary = input.directions.secondary = glm::vec2(0.0f);
        for (int i=0; i<4; i++) {
            input.actions[i] = InputState::None;
        }
    }

    struct {
        struct {
            glm::vec2 primary;
            glm::vec2 secondary;
        } directions;
        InputState::Enum actions[4];
    } input;

    glm::vec2 facingDirection;
};

#define thePlayerSystem PlayerSystem::GetInstance()
#if SAC_DEBUG
#define PLAYER(e) thePlayerSystem.Get(e, true, __FILE__, __LINE__)
#else
#define PLAYER(e) thePlayerSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Player)

public:
    PrototypeGame* game;
}
;
