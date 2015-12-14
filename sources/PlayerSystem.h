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

namespace ScoreRule {
    enum Enum {
        Bet = 0,
        Jordane
    };
}

struct PlayerComponent {
    PlayerComponent() {
    }

    struct {
        InputState::Enum directions[4];
        InputState::Enum actions[1];
    } input;

    struct {
        int bet;
        int coinCount;
        int score;
        ScoreRule::Enum rule;
    } score[10]; // assume 10 round game
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
