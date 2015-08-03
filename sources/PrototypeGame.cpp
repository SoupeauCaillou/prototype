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
#include "util/Random.h"

#include "systems/AnchorSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/CameraSystem.h"
#include "systems/TransformationSystem.h"

#include "base/TimeUtil.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

PrototypeGame::PrototypeGame() : Game() {
    registerScenes(this, sceneStateMachine);
}

Entity player;

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "idle", "idle");
    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "run", "run");

    CAMERA(camera)->clearColor = Color(0.415, 0.745, 0.188);

    player = theEntityManager.CreateEntityFromTemplate("player");
    Entity shadow = theEntityManager.CreateEntityFromTemplate("shadow");
    ANCHOR(shadow)->parent = player;

    sceneStateMachine.start(Scene::Menu);

    float b_pos[200];
    Random::N_Floats(200, b_pos, -20, 20);
    for (int i = 0; i < 100; i++) {
        Entity b = theEntityManager.CreateEntityFromTemplate("brush");
        TRANSFORM(b)->position = glm::vec2(b_pos[2 * i], b_pos[2 * i + 1]);
    }
}

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);

    float runningSpeed = 0.0f;

    if (theTouchInputManager.isTouched()) {
        runningSpeed = 2.5f;
    } else if (ANIMATION(player)->name == HASH("run", 0xf665a795) &&
               RENDERING(player)->texture != HASH("run2", 0x11401477) &&
               RENDERING(player)->texture != HASH("run5", 0xe188f3d5)) {
        runningSpeed = 0.5;
    } else {
        runningSpeed = 0.0f;
    }

    if (runningSpeed > 0) {
        glm::vec2 dir = theTouchInputManager.getTouchLastPosition() -
                        TRANSFORM(player)->position;
        TRANSFORM(player)->position += runningSpeed * dt * glm::normalize(dir);
        ANIMATION(player)->name = HASH("run", 0xf665a795);
        if (dir.x < 0) {
            RENDERING(player)->flags |= RenderingFlags::MirrorHorizontal;
        } else {
            RENDERING(player)->flags &= ~RenderingFlags::MirrorHorizontal;
        }
    } else {
        ANIMATION(player)->name = HASH("idle", 0xed137eaa);
    }

    glm::vec2 diff = TRANSFORM(player)->position - TRANSFORM(camera)->position;
    float l = glm::length(diff);
    TRANSFORM(camera)->position += diff * glm::min(1.5f, l) * dt;
}
