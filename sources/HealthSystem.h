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

struct HealthComponent {
    HealthComponent() {
        maxHP = currentHP = 0; hitBy = 0;
    }

    int maxHP;
    int currentHP;
    Entity hitBy;
};

#define theHealthSystem HealthSystem::GetInstance()
#if SAC_DEBUG
#define HEALTH(e) theHealthSystem.Get(e, true, __FILE__, __LINE__)
#else
#define HEALTH(e) theHealthSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Health)

public:
    PrototypeGame* game;
}
;
