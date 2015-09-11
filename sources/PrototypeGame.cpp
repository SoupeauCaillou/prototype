/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

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

#include "PrototypeGame.h"

#include "base/PlacementHelper.h"
#include "base/StateMachine.inl"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"

#include "systems/CameraSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"

#include "base/TimeUtil.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

Entity vehicle;

PrototypeGame::PrototypeGame() : Game() {
    registerScenes(this, sceneStateMachine);
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    sceneStateMachine.start(Scene::Menu);

    theAnimationSystem.loadAnim(gameThreadContext->assetAPI,
        "bulldo_idle", "bulldo_idle");
    theAnimationSystem.loadAnim(gameThreadContext->assetAPI,
        "bulldo_move", "bulldo_move");

    vehicle = theEntityManager.CreateEntityFromTemplate("vehicle");

    ADD_COMPONENT(camera, Physics);
    PHYSICS(camera)->mass = 1;
}

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);

    if (theTouchInputManager.isTouched()) {
        PHYSICS(vehicle)->addForce(
            Force(
                glm::vec2(tuning.f(HASH("move_force", 0x476a8439)), 0),
                glm::vec2(0.0f)),
            0.016);
    }

    if (PHYSICS(vehicle)->linearVelocity.x >
        tuning.f(HASH("min_speed_animation", 0x8d3ec027))) {
        ANIMATION(vehicle)->name = HASH("bulldo_move", 0xa7240136);
    } else {
        ANIMATION(vehicle)->name = HASH("bulldo_idle", 0x96648810);
    }

    // objective: keep the vehicle at 2/3x
    float obj = TRANSFORM(camera)->position.x +
        TRANSFORM(camera)->size.x * (-1/2.0f + 1/3.0f);

    float diff = TRANSFORM(vehicle)->position.x - obj;

    TRANSFORM(camera)->position.x +=
        diff * dt * tuning.f(HASH("camera_update_speed", 0xecef88a5));
}
