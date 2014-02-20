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

namespace Status
{
    enum Enum {
        Idle,
        Moving,
        Bouncing,
        CanAttack,
        Attack,
        CoolDown,
    };
}

struct SoldierComponent {
    SoldierComponent(): player (0), health(1.0f), brakingForce(-5.0f), maxSpeedCollision(0.1), status(Status::Idle), flickingDistance(0.0f) { cd.accum = 0; cd.duration = 1.0f;}

    Entity player;
    float health;
    float brakingForce;
    float maxSpeedCollision;
    Status::Enum status;
    float flickingDistance;
    struct {
        float accum;
        float duration;
    } cd;
};

#define theSoldierSystem SoldierSystem::GetInstance()
#if SAC_DEBUG
#define SOLDIER(e) theSoldierSystem.Get(e,true,__FILE__,__LINE__)
#else
#define SOLDIER(e) theSoldierSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Soldier)
};
