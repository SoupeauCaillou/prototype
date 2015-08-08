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
#include "util/IntersectionUtil.h"
#include "util/Tuning.h"

#include "systems/AnchorSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/CameraSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"

#include "ShadowSystem.h"

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
Entity ball;
Entity hitzone;
Entity plot;

void addShadow(PrototypeGame* game, Entity e) {
    Entity shadow = theEntityManager.CreateEntityFromTemplate("shadow");
    ANCHOR(shadow)->parent = e;
    ANCHOR(shadow)->position.y = -TRANSFORM(e)->size.y * 0.5;
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    ShadowSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(ShadowSystem::GetInstancePointer());

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "idle", "idle");
    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "run", "run");
    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "tackle", "tackle");

    CAMERA(camera)->clearColor = Color(0.415, 0.745, 0.188);

    player = theEntityManager.CreateEntityFromTemplate("player");
    hitzone = theEntityManager.CreateEntityFromTemplate("player_hitzone");
    ball = theEntityManager.CreateEntityFromTemplate("ball");
    plot = theEntityManager.CreateEntityFromTemplate("plot");

    ANCHOR(player)->parent =
        hitzone;

    addShadow(this, player);
    addShadow(this, ball);
    addShadow(this, plot);


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

    // joystick control
    glm::vec2 dir = gameThreadContext->joystickAPI->getPadDirection(0, 0);
    float dirLength = glm::length(dir);
    // mouse control override
    if (theTouchInputManager.isTouched()) {
        dir = theTouchInputManager.getTouchLastPosition() -
              TRANSFORM(hitzone)->position;
        dirLength = 1.0f;
    }

    if (dirLength > 0.1) {
        nextAction = actions::Run;
    } else {
        nextAction = actions::Idle;
    }
    // A button or right click -> tackle
    if (gameThreadContext->joystickAPI->hasClicked(0, 0) ||
        theTouchInputManager.hasClicked(1)) {
        nextAction = actions::Tackle;
    }

    // Allow new actions only at certain key-frames of animations
    if (!canChangeAction(player)) {
        // prevent direction change during non-running actions
        if (currentAction != actions::Run) {
            dir = previousDir;
        }
    } else {
        currentAction = nextAction;
    }

    switch(currentAction) {
    case actions::Idle:
        runningSpeed = 0.0f;
        ANIMATION(player)->name = HASH("idle", 0xed137eaa);
        break;
    case actions::Run:
        runningSpeed = tuning.f(HASH("running_speed", 0x1f34eb01));
        ANIMATION(player)->name = HASH("run", 0xf665a795);
        break;
    case actions::Tackle:
        runningSpeed = tuning.f(HASH("tackle_speed", 0));
        ANIMATION(player)->name = HASH("tackle", 0x79891832);
        dir = previousDir;
        break;
    }

    bool touchBall = IntersectionUtil::rectangleRectangle(TRANSFORM(ball), TRANSFORM(hitzone));
    const float kickForce = tuning.f(HASH("kick_force", 0xde7152e1));

    // kick the ball
    if (currentAction == actions::Run && dirLength > 0) {
        static bool kickEnabled = true;

        if (kickEnabled) {
            if (touchBall) {
                LOGI(dir);
                PHYSICS(ball)->addForce(
                    Force(glm::normalize(dir) * kickForce,glm::vec2(0.0f)), 0.016f);
                kickEnabled = false;
            }
        } else {
            if (!touchBall) {
                kickEnabled = true;
            }
        }
        RENDERING(hitzone)->color = Color(!kickEnabled, 0, kickEnabled);
    }

    if (runningSpeed > 0) {
        if (gameThreadContext->joystickAPI->isDown(0, 1)) {
            // lock direction until contact
            if (!touchBall) {
                dir = glm::normalize(
                    TRANSFORM(ball)->position -
                    TRANSFORM(hitzone)->position);
            } else {
                LOGI("Ok, new dir " << __(dir));
            }
        }

        if (dir.x || dir.y) {
            TRANSFORM(hitzone)
                ->position +=
                runningSpeed * dt * glm::normalize(dir) * glm::min(dirLength, 1.0f);
            LOGV(1, __(TRANSFORM(hitzone)->position) << __(dir) << __(dirLength) << __(previousDir));
        }


        if (dir.x < 0) {
            RENDERING(player)->flags |= RenderingFlags::MirrorHorizontal;
        } else {
            RENDERING(player)->flags &= ~RenderingFlags::MirrorHorizontal;
        }
    }

    // adjust camera position
    glm::vec2 diff = TRANSFORM(hitzone)->position - TRANSFORM(camera)->position;
    float l = glm::length(diff);
    TRANSFORM(camera)->position += diff * glm::min(1.5f, l) * dt;
    previousDir = dir;
}
