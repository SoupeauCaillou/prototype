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



#include "ArcherSystem.h"

#include "FlickSystem.h"
#include "SoldierSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

INSTANCE_IMPL(ArcherSystem);

ArcherSystem::ArcherSystem() : ComponentSystemImpl<ArcherComponent>("Archer") {
    ArcherComponent tc;
    componentSerializer.add(new Property<float>("range_coeff", OFFSET(rangeCoeff, tc)));
    componentSerializer.add(new Property<float>("arrow_force", OFFSET(arrowForce, tc)));
}

void ArcherSystem::DoUpdate(float) {
    for (auto p: components) {
        Entity e = p.first;
        auto* kc = p.second;
        auto* sc = SOLDIER(e);
        if (sc->status == Status::Attack) {
            const float atkRange = sc->flickingDistance * kc->rangeCoeff;
            LOGI("ATK range: " << atkRange);
            const auto* tc = TRANSFORM(e);

            // find nearest target within range
            Entity target = 0;
            float nearest = -1;
            theSoldierSystem.forEachECDo([tc, kc, sc, e, &target, &nearest, atkRange] (Entity f, SoldierComponent* sc2) -> void {
                if (sc2->health <= 0 || f == e || sc->player == sc2->player)
                    return;

                const float d = glm::distance(tc->position, TRANSFORM(f)->position);

                if (d <= atkRange && (d < nearest || target == 0)) {
                    nearest = d;
                    target = f;
                }
            });

            if (target) {
                LOGI("Archer " << theEntityManager.entityName(e) << " aims " << theEntityManager.entityName(target));

                // fire arrow
                glm::vec2 nDif = glm::normalize(TRANSFORM(target)->position - TRANSFORM(e)->position);
                const float angle = glm::atan(nDif.y, nDif.x);
                Entity arrow = theEntityManager.CreateEntityFromTemplate("game/arrow");
                TRANSFORM(arrow)->position = TRANSFORM(e)->position + nDif * (glm::length(TRANSFORM(e)->size) + glm::length(TRANSFORM(arrow)->size)) * 0.6f;
                TRANSFORM(arrow)->rotation = angle;
                PHYSICS(arrow)->addForce(nDif * kc->arrowForce, glm::vec2(0.0f), 0.016);
            }
        }
    }
}

