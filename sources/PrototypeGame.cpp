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
#include "api/JoystickAPI.h"

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
    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "tackle", "tackle");

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

bool canChangeAction(Entity p) {
    switch (ANIMATION(player)->name) {
        case 0xf665a795: /* run */
            return
                RENDERING(player)->texture == HASH("run2", 0x11401477) ||
                RENDERING(player)->texture == HASH("run5", 0xe188f3d5);

        case 0xed137eaa: /* idle */
            return true;

        case 0x79891832: /* tackle */
            return
                RENDERING(player)->texture == HASH("tackle4", 0x4eed9509);
        default:
            return true;
    }
}

namespace actions {
    enum Enum {
        Idle,
        Run,
        Tackle
    };
}

glm::vec2 previousDir;
actions::Enum currentAction = actions::Idle;

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);

    float runningSpeed = 0.0f;

    actions::Enum nextAction = actions::Idle;

    glm::vec2 dir = gameThreadContext->joystickAPI->getPadDirection(0, 0);
    float dirLength = glm::length(dir);

    if (theTouchInputManager.isTouched() || dirLength > 0.1) {
        nextAction = actions::Run;
    } else {
        nextAction = actions::Idle;
    }
    if (gameThreadContext->joystickAPI->hasClicked(0, 0)) {
        nextAction = actions::Tackle;
    }

    if (!canChangeAction(player)) {
        dir = previousDir;
    } else {
        currentAction = nextAction;
    }

    switch(currentAction) {
    case actions::Idle:
        runningSpeed = 0.0f;
        ANIMATION(player)->name = HASH("idle", 0xed137eaa);
        break;
    case actions::Run:
        runningSpeed = 3.5f;
        ANIMATION(player)->name = HASH("run", 0xf665a795);
        break;
    case actions::Tackle:
        runningSpeed = 2.5f;
        ANIMATION(player)->name = HASH("tackle", 0x79891832);
        dir = previousDir;
        break;
    }

    if (runningSpeed > 0) {
        if (theTouchInputManager.isTouched()) {
            dir = theTouchInputManager.getTouchLastPosition() -
                  TRANSFORM(player)->position;
            dirLength = 1.0f;
        }

        TRANSFORM(player)
            ->position +=
            runningSpeed * dt * glm::normalize(dir) * glm::min(dirLength, 1.0f);


        if (dir.x < 0) {
            RENDERING(player)->flags |= RenderingFlags::MirrorHorizontal;
        } else {
            RENDERING(player)->flags &= ~RenderingFlags::MirrorHorizontal;
        }
    }

    glm::vec2 diff = TRANSFORM(player)->position - TRANSFORM(camera)->position;
    float l = glm::length(diff);
    TRANSFORM(camera)->position += diff * glm::min(1.5f, l) * dt;
    previousDir = dir;
}
