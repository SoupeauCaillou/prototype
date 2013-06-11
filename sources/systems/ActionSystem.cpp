/*
    This file is part of Prototype.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ActionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/SoldierSystem.h"
#include "systems/PlayerSystem.h"
#include "../PrototypeGame.h"

#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(ActionSystem);

ActionSystem::ActionSystem() : ComponentSystemImpl<ActionComponent>("Action") {
    ActionComponent sc;
}

static void payForAction(Entity soldier, Action::Enum type) {
    int actionCost = 0;
    switch (type) {
        case Action::None:
            break;
        case Action::MoveTo:
            actionCost = 1;
            break;
        case Action::Attack:
            actionCost = 2;
            break;
    }

    PLAYER(SOLDIER(soldier)->player)->actionPointsLeft -= actionCost;
    SOLDIER(soldier)->actionPointsLeft -= actionCost;
}

void ActionSystem::DoUpdate(float dt) {
    std::vector<Entity> actionFinished;
    std::vector<ActionComponent*> actionDependingOnAnother;

    for (auto& p: components) {
        Entity entity = p.first;
        ActionComponent* ac = p.second;

        if (ac->dependsOn <= 0) {
            switch (ac->type) {
                case Action::None: {
                    // mark action as finished
                    actionFinished.push_back(entity);
                    break;
                }
                case Action::MoveTo: {
                    glm::vec2 diff (ac->moveToTarget - TRANSFORM(ac->entity)->position);
                    float length = glm::length(diff);
                    if (length < 0.001 || length <= ac->moveSpeed * dt) {
                        // set final position
                        TRANSFORM(ac->entity)->position = ac->moveToTarget;
                        // mark action as finished
                        actionFinished.push_back(entity);
                    } else {
                        // move nearer to target
                        TRANSFORM(ac->entity)->position += diff * (ac->moveSpeed * dt) / length;
                        // rotate
                        TRANSFORM(ac->entity)->rotation = glm::atan2(diff.y, diff.x);
                    }
                    break;
                }
                case Action::Attack: {
                    static float accum = 0;
                    if (accum == 0.0f) {
                        LOGI("Resolve attack");
                        const auto* sc = SOLDIER(ac->entity);
                        // Resolve attack
                        //   1. determine distance
                        unsigned distance = game->grid.computeGridDistance(
                            TRANSFORM(ac->entity)->position,
                            TRANSFORM(ac->attackTarget)->position);
                        LOGI("Distance: " << distance);
                        //   2. if closer than min distance, use min distance
                        distance = glm::max(distance, sc->attackRange.t1);
                        LOGI("Distance bis: " << distance);
                        //   3. compute hit probability
                        float hitProbability = glm::lerp(0.9f, 0.1f, distance / (float)sc->attackRange.t2);
                        LOGI("Proba: " << hitProbability);
                        //   4. throw 1d10!
                        float dice = glm::linearRand(0.0f, 1.0f);
                        LOGI("Dice: " << dice);
                        //   5. It's a hit ?!
                        if (dice <= hitProbability) {
                            LOGI(theEntityManager.entityName(ac->entity) << " just hit " <<
                                theEntityManager.entityName(ac->attackTarget));
                            SOLDIER(ac->attackTarget)->maxActionPointsPerTurn
                                -= SOLDIER(ac->entity)->attackDamage;
                        }
                    }

                    accum += dt;
                    if (accum >= 1) {
                        accum = 0;
                        // mark action as finished
                        actionFinished.push_back(entity);
                        PLAYER(SOLDIER(ac->entity)->player)->actionPointsLeft-=2;
                    }
                    break;
                }
            }
        } else {
            actionDependingOnAnother.push_back(ac);
        }
    }

    if (!actionFinished.empty()) {
        for (auto eDone: actionFinished) {
            payForAction(ACTION(eDone)->entity, ACTION(eDone)->type);

            if (!actionDependingOnAnother.empty()) {
                for (auto aBlocked: actionDependingOnAnother) {
                    if (aBlocked->dependsOn == eDone) {
                        aBlocked->dependsOn = 0;
                        break;
                    }
                }
            }
            theEntityManager.DeleteEntity(eDone);
        }
    }


}
