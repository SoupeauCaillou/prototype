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



#include "SheepSystem.h"

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"

#include <glm/gtx/norm.hpp>


INSTANCE_IMPL(SheepSystem);

SheepSystem::SheepSystem() : ComponentSystemImpl<SheepComponent>(HASH("Sheep", 0x44d2846b)) {
    SheepComponent sc;

    componentSerializer.add(new Property<float>(HASH("alignement_min_distance", 0xe06b6bdc), OFFSET(alignementMinDistance, sc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("cohesion_min_distance", 0xc59de7f1), OFFSET(cohesionMinDistance, sc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("separation_min_distance", 0xd2d08763), OFFSET(separationMinDistance, sc), 0.001f));
}

void SheepSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(SheepComponent, sheep, sc)
        AutonomousAgentComponent* aac = AUTONOMOUS(sheep);

        glm::vec2 & myPos = TRANSFORM(sheep)->position;

        aac->cohesionNeighbors.clear();
        aac->alignementNeighbors.clear();
        aac->separationNeighbors.clear();

        FOR_EACH_ENTITY_COMPONENT(SheepComponent, anotherSheep, scN)
            if (anotherSheep == sheep)
                continue;

            if (glm::distance2(myPos, TRANSFORM(anotherSheep)->position) < sc->alignementMinDistance * sc->alignementMinDistance) {
                aac->alignementNeighbors.push_back(anotherSheep);
            }
            if (glm::distance2(myPos, TRANSFORM(anotherSheep)->position) < sc->cohesionMinDistance * sc->cohesionMinDistance) {
                aac->cohesionNeighbors.push_back(anotherSheep);
            }
            if (glm::distance2(myPos, TRANSFORM(anotherSheep)->position) < sc->separationMinDistance * sc->separationMinDistance) {
                aac->separationNeighbors.push_back(anotherSheep);
            }
        }
    }
}
