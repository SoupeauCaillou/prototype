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



#include "FlickSystem.h"
#include "base/TouchInputManager.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

#include "util/DrawSomething.h"

INSTANCE_IMPL(FlickSystem);

FlickSystem::FlickSystem() : ComponentSystemImpl<FlickComponent>("Flick") {
    FlickComponent tc;
    componentSerializer.add(new Property<float>("max_force", OFFSET(maxForce, tc)));
    componentSerializer.add(new Property<float>("min_distance", OFFSET(activationDistance.t1, tc)));
    componentSerializer.add(new Property<float>("max_distance", OFFSET(activationDistance.t2, tc)));
    componentSerializer.add(new Property<bool>("enabled", OFFSET(enabled, tc)));
    componentSerializer.add(new Property<int>("status", OFFSET(status, tc)));
}

void FlickSystem::DoUpdate(float dt) {
    const bool touching = theTouchInputManager.isTouched(0);

    DrawSomething::DrawVec2Restart("flick");

    for (auto cpt: components) {
        Entity e = cpt.first;
        auto* fc = cpt.second;

        if (!fc->enabled) {
            fc->status = FlickStatus::NotPossible;
            continue;
        }

        if (fc->status == FlickStatus::NotPossible ||
            (fc->status == FlickStatus::Moving && glm::length(PHYSICS(e)->linearVelocity) < 0.1)) {
            fc->status = FlickStatus::Idle;
        }

        if (fc->status == FlickStatus::Idle && !touching) {
            continue;
        }

        if (fc->status == FlickStatus::UserInput) {
            glm::vec2 diff (theTouchInputManager.getTouchLastPosition() - TRANSFORM(e)->position);
            float l = glm::length(diff);
            if (l > fc->activationDistance.t2) {
                diff *= fc->activationDistance.t2 / l;
                l = fc->activationDistance.t2;
            }

            if (touching) {
                // update UI
                DrawSomething::DrawVec2("flick", TRANSFORM(e)->position, diff, false);
            } else {
                // apply force
                fc->status = FlickStatus::Moving; // (transitory state)
                if (l >= fc->activationDistance.t1) {
                    float forceMag = glm::lerp(0.0f, fc->maxForce, (l - fc->activationDistance.t1) / (fc->activationDistance.t2 - fc->activationDistance.t1));
                    PHYSICS(e)->addForce(diff * forceMag / l, glm::vec2(0.0f), 0.016);
                }
            }
        } else {
            if (!theTouchInputManager.wasTouched(0)) {
                if (glm::distance(theTouchInputManager.getTouchLastPosition(), TRANSFORM(e)->position) <= fc->activationDistance.t1) {
                    fc->status = FlickStatus::UserInput;
                }
            }
        }
    }
}

