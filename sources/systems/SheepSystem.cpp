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

SheepSystem::SheepSystem() : ComponentSystemImpl<SheepComponent>("Sheep") {
    SheepComponent sc;

    componentSerializer.add(new Property<float>("alignement_min_distance", OFFSET(alignementMinDistance, sc), 0.001f));
    componentSerializer.add(new Property<float>("cohesion_min_distance", OFFSET(cohesionMinDistance, sc), 0.001f));
    componentSerializer.add(new Property<float>("separation_min_distance", OFFSET(separationMinDistance, sc), 0.001f));
}

void SheepSystem::DoUpdate(float) {
    for (auto& p: components) {
        Entity sheep = p.first;
        SheepComponent* sc = p.second;
        
        AutonomousAgentComponent* aac = AUTONOMOUS(sheep);

        glm::vec2 & myPos = TRANSFORM(sheep)->position;

        aac->cohesionNeighbors.clear();
        aac->alignementNeighbors.clear();
        aac->separationNeighbors.clear();

        for (auto& sheepN: components) {
            Entity anotherSheep = sheepN.first;
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
