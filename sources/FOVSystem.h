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
#include "systems/opengl/Polygon.h"

struct FOVComponent {
    FOVComponent() {
        rayIndexes[0] = rayIndexes[1] = -1;
    }

    std::vector<Polygon> polygons;
    int groupsImpactingFOV;

    // private
    glm::vec2 reference;
    int rayIndexes[2];
};

#define theFOVSystem FOVSystem::GetInstance()
#if SAC_DEBUG
#define FOV(e) theFOVSystem.Get(e,true,__FILE__,__LINE__)
#else
#define FOV(e) theFOVSystem.Get(e)
#endif

UPDATABLE_SYSTEM(FOV)

private:
    std::vector<Entity> rays;
};
