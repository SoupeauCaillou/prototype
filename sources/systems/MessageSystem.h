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
#include "../states/Scenes.h"

namespace MessageType
{
    enum Enum {
        None,
        NewScene,
        Flick,
    };
}

struct MessageComponent {
    MessageComponent(): type(MessageType::None), ttl(1) {
        flick.target = 0;
        flick.force = glm::vec2(0.0f);
    }

    MessageType::Enum type;
    Scene::Enum newScene;
    struct {
        Entity target;
        glm::vec2 force;
    } flick;

    int ttl;
};

#define theMessageSystem MessageSystem::GetInstance()
#if SAC_DEBUG
#define MESSAGE(e) theMessageSystem.Get(e,true,__FILE__,__LINE__)
#else
#define MESSAGE(e) theMessageSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Message)
};
