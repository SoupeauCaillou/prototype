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
#include "systems/AnchorSystem.h"

#include "base/TimeUtil.h"

#include "util/Random.h"
#include "util/IntersectionUtil.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

Entity vehicle, smoke;
int lastBackgroundX;

glm::vec3 coeffA = {.5, .5, .5};
glm::vec3 coeffB = {.5, .5, .5};
glm::vec3 coeffC = {1/20., .7/20., .4/20.};
glm::vec3 coeffD = {0., .15, .2};

PrototypeGame::PrototypeGame() : Game() { registerScenes(this, sceneStateMachine); }

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
    smoke = theEntityManager.CreateEntityFromTemplate("smoke");
    ANCHOR(smoke)->parent = vehicle;


    lastBackgroundX = TRANSFORM(camera)->position.x - (5 + TRANSFORM(camera)->size.x / 2.f);
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


    float i = lastBackgroundX;
    while (i < TRANSFORM(camera)->position.x + TRANSFORM(camera)->size.x / 2.f + 3) {
        const auto & size = PlacementHelper::ScreenSize;

        Entity e = theEntityManager.CreateEntityFromTemplate("background");
        TRANSFORM(e)->position.x = i;
        for (int j = 0; j < 3; j++) {
            RENDERING(e)->color.rgba[j] = coeffA[j] + coeffB[j] * cos(2 * M_PI * ((int)i%20 * coeffC[j] + coeffD[j]));
        }

        if (Random::Int(0, 100) > 60) {
            Entity tree = theEntityManager.CreateEntityFromTemplate("burning_tree");
            TRANSFORM(tree)->position.x = TRANSFORM(e)->position.x + TRANSFORM(e)->size.x / 2.f;
        }
        i += TRANSFORM(e)->size.x;
    }
    lastBackgroundX = i;
}
