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
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(ActionSystem);

ActionSystem::ActionSystem() : ComponentSystemImpl<ActionComponent>("Action") {
    ActionComponent sc;
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
                        LOGI(TRANSFORM(ac->entity)->position);
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
            }
        } else {
            actionDependingOnAnother.push_back(ac);
        }
    }

    if (!actionFinished.empty()) {
        for (auto eDone: actionFinished) {
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
