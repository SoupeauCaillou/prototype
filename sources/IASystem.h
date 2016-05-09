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

namespace IAState {
    enum Enum {
        Idle = 0,
        Aiming,
        Moving,
        Count
    };
}

struct IAComponent {

    IAComponent() : state (IAState::Idle), stateDuration(0) {}

    IAState::Enum state;
    float stateDuration;

    float idleDuration;
    float moveDuration;
    glm::vec2 moveDirection;

    /* escape from players */
    struct {
        float minDistance;
        float weight;
    } espace;

    /* regroup with friends */
    struct {
        float weight;
    } regroup;
};

#define theIASystem IASystem::GetInstance()
#if SAC_DEBUG
#define IA(e) theIASystem.Get(e, true, __FILE__, __LINE__)
#else
#define IA(e) theIASystem.Get(e)
#endif

UPDATABLE_SYSTEM(IA)

public:
    PrototypeGame* game;
}
;
