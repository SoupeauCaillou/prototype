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



#include "KnightSystem.h"

#include "FlickSystem.h"
#include "SoldierSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

INSTANCE_IMPL(KnightSystem);

KnightSystem::KnightSystem() : ComponentSystemImpl<KnightComponent>("Knight") {
    KnightComponent tc;
    componentSerializer.add(new Property<float>("attack_range", OFFSET(attackRange, tc)));
    componentSerializer.add(new Property<float>("attack_coeff", OFFSET(attackCoeff, tc)));
}

void KnightSystem::DoUpdate(float) {
    for (auto p: components) {
        Entity e = p.first;
        auto* kc = p.second;
        auto* sc = SOLDIER(e);
        if (sc->status == Status::Attack) {
            const auto* tc = TRANSFORM(e);

            std::vector<Entity> targets;
            // attack all targets within range
            theSoldierSystem.forEachECDo([tc, sc, kc, e, &targets] (Entity f, SoldierComponent* sc2) -> void {
                if (sc2->health <= 0 || f == e || sc->player == sc2->player)
                    return;

                if (glm::distance(tc->position, TRANSFORM(f)->position) <= kc->attackRange) {
                    targets.push_back(f);
                }
            });

            if (!targets.empty()) {
                float damage = sc->flickingDistance * kc->attackCoeff / targets.size();
                for (Entity f: targets) {
                    LOGI("Knight " << theEntityManager.entityName(e) << " attacks " << theEntityManager.entityName(f) << ": " << damage) ;
                    auto* sc2 = SOLDIER(f);
                    sc2->health = glm::max(0.0f, sc2->health - damage);
                }
            }
        }
    }
}

