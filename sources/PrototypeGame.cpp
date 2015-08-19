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
#include "systems/CollisionSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"

#include "ShadowSystem.h"

#include "base/TimeUtil.h"
#include "base/Interval.h"
#include "api/JoystickAPI.h"
#include "api/KeyboardInputHandlerAPI.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

PrototypeGame::PrototypeGame() : Game() { registerScenes(this, sceneStateMachine); }

namespace actions {
    enum Enum { Idle, Run, Tackle, Walk };
}

struct Player {
    Player() {
        render = hitzone = 0;
        previousDir = glm::vec2(0.0f);
        currentAction = actions::Idle;
        joystick = -1;
    }

    Entity render;
    Entity hitzone;

    glm::vec2 previousDir;
    actions::Enum currentAction;

    int joystick;
};
std::vector<Player> players;

Entity ball, ballHitzone;
std::vector<Entity> plots;
std::vector<Entity> hitzones;

Entity lastPlayerWhoKickedTheBall = 0;

void addHitzone(PrototypeGame* game, Entity e) {
    Entity h = theEntityManager.CreateEntityFromTemplate("hitzone");
    ANCHOR(h)->parent = e;
    TRANSFORM(h)->size = glm::vec2(TRANSFORM(e)->size.x);

    hitzones.push_back(h);

    if (e == ball) {
        COLLISION(h)->group = 2;
        COLLISION(h)->collideWith = 0xffffffff;
        ballHitzone = h;
    }
}

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
    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "walk", "walk");
    theAnimationSystem.loadAnim(gameThreadContext->assetAPI, "tackle", "tackle");

    CAMERA(camera)->clearColor = Color(0.415, 0.745, 0.188);

    theCollisionSystem.worldSize = glm::vec2(20, 20);

    // load entity from entity_desc.ini file
    FileBuffer fb = gameThreadContext->assetAPI->loadAsset("entity_desc.ini");
    if (fb.size) {
        DataFileParser dfp;
        dfp.load(fb, "entity_desc.ini");
        int count = 0;
        dfp.get(DataFileParser::GlobalSection, "count", &count);
        std::vector<Entity> all;
        for (int i = 0; i < count; i++) {
            char tmp[128];
            sprintf(tmp, "%d", i + 1);
            hash_t section = Murmur::RuntimeHash(tmp);

            std::string entity;
            glm::vec2 position;
            int noShadow = 0;
            dfp.get(section, HASH("type", 0xf3ebd1bf), &entity);
            dfp.get(section, HASH("position", 0xffab91ef), &position.x, 2);
            dfp.get(section, HASH("no_shadow", 0x4370b3f1), &noShadow, 1);

            if (entity == "player") {
                Player p;
                p.render = theEntityManager.CreateEntityFromTemplate("player");
                p.hitzone = theEntityManager.CreateEntityFromTemplate("player_hitzone");
                TRANSFORM(p.hitzone)->position = position;
                ANCHOR(p.render)->parent = p.hitzone;
                addShadow(this, p.render);
                players.push_back(p);

                all.push_back(p.render);
            } else {
                Entity e = theEntityManager.CreateEntityFromTemplate(entity.c_str());
                TRANSFORM(e)->position = position;
                if (!noShadow) {
                    addShadow(this, e);
                }

                if (entity == "plot")
                    plots.push_back(e);
                else if (entity == "ball")
                    ball = e;

                addHitzone(this, e);
                all.push_back(e);
            }
        }
    }

    players[0].joystick = 0;

    sceneStateMachine.start(Scene::Menu);

    float b_pos[200];
    Random::N_Floats(200, b_pos, -20, 20);
    for (int i = 0; i < 100; i++) {
        Entity b = theEntityManager.CreateEntityFromTemplate("brush");
        TRANSFORM(b)->position = glm::vec2(b_pos[2 * i], b_pos[2 * i + 1]);
    }
}

static bool canChangeAction(Entity p) {
    switch (ANIMATION(p)->name) {
        case 0x5787408a: /* walk */
            return true;
        case 0xf665a795: /* run */
            return RENDERING(p)->texture == HASH("run2", 0x11401477) ||
                   RENDERING(p)->texture == HASH("run5", 0xe188f3d5);

        case 0xed137eaa: /* idle */
            return true;

        case 0x79891832: /* tackle */
            return RENDERING(p)->texture == HASH("tackle4", 0x4eed9509);
        default:
            return true;
    }
}

static void adjustZWithOnScreenPosition(Game* game, Entity e, const Interval<float>& camera) {
    auto* tc = TRANSFORM(e);
    float t = camera.invLerp(tc->position.y);
    Interval<float> z(game->tuning.f(HASH("adjust_min_z", 0x533b118a)),
                      game->tuning.f(HASH("adjust_max_z", 0x583d04f9)));
    tc->z = z.lerp(t);
}

bool updateControlledPlayer(struct Player& player, float dt, Game* game) {
    LOGF_IF(player.joystick < 0, "Invalid joystick");

    bool mouseControlAllowed = (player.joystick == 0);
    float runningSpeed = 0.0f;

    actions::Enum nextAction = actions::Idle;

    // joystick control
    glm::vec2 dir = game->gameThreadContext->joystickAPI->getPadDirection(player.joystick, 0);
    bool sprint = game->gameThreadContext->joystickAPI->isDown(player.joystick, 5) ||
                  game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(' ');
    float dirLength = glm::length(dir);
    if (dirLength) {
        dir /= dirLength;
    }

    // mouse control override
    if (mouseControlAllowed && theTouchInputManager.isTouched()) {
        dir = glm::normalize(theTouchInputManager.getTouchLastPosition() - TRANSFORM(player.hitzone)->position);
        dirLength = 1.0f;
    }

    if (dirLength > 0.1) {
        nextAction = sprint ? actions::Run : actions::Walk;
    } else {
        nextAction = actions::Idle;
    }
    // A button or right click -> tackle
    if (game->gameThreadContext->joystickAPI->hasClicked(player.joystick, 0) ||
        (mouseControlAllowed && theTouchInputManager.hasClicked(1))) {
        nextAction = actions::Tackle;
    }

    // Allow new actions only at certain key-frames of animations
    if (!canChangeAction(player.render)) {
        // prevent direction change during non-running actions
        if (player.currentAction != actions::Run && player.currentAction != actions::Walk) {
            dir = player.previousDir;
            dirLength = glm::length(dir);
        }
    } else {
        player.currentAction = nextAction;
    }

    switch (player.currentAction) {
        case actions::Idle:
            runningSpeed = 0.0f;
            ANIMATION(player.render)->name = HASH("idle", 0xed137eaa);
            break;
        case actions::Walk:
            runningSpeed = game->tuning.f(HASH("walking_speed", 0x73dcc3ec));
            ANIMATION(player.render)->name = HASH("walk", 0x5787408a);
            break;
        case actions::Run:
            runningSpeed = game->tuning.f(HASH("running_speed", 0x1f34eb01));
            ANIMATION(player.render)->name = HASH("run", 0xf665a795);
            break;
        case actions::Tackle:
            runningSpeed = game->tuning.f(HASH("tackle_speed", 0xf53acc8e));
            ANIMATION(player.render)->name = HASH("tackle", 0x79891832);
            dir = player.previousDir;
            break;
    }

    bool touchBall = IntersectionUtil::rectangleRectangle(TRANSFORM(ball), TRANSFORM(player.hitzone));

    // kick the ball
    if ((player.currentAction == actions::Run || player.currentAction == actions::Walk) && dirLength > 0) {
        const float kickForce = game->tuning.f(HASH("kick_force_slow", 0x3987715d));

        static bool kickEnabled = true;

        // only kick if ball is going slower and/or in the wrong direction
        if (kickEnabled) {
            if (touchBall) {
                glm::vec2 desiredBallPosition =
                    TRANSFORM(player.hitzone)->position +
                    dir * runningSpeed * game->tuning.f(HASH("ball_advance_lookup_sec", 0xcf261ebb));

                glm::vec2 diff = desiredBallPosition - TRANSFORM(ball)->position;

                PHYSICS(ball)->linearVelocity = glm::vec2(0.0f);
                PHYSICS(ball)->addForce(Force(glm::length(diff) * kickForce * dir, glm::vec2(0.0f)), 0.016f);
                kickEnabled = false;

                lastPlayerWhoKickedTheBall = player.render;
            }
        } else {
            if (!touchBall) {
                kickEnabled = true;
            }
        }
        RENDERING(player.hitzone)->color = Color(!kickEnabled, 0, kickEnabled);
    }
    if (lastPlayerWhoKickedTheBall == player.render &&
        game->gameThreadContext->joystickAPI->isDown(player.joystick, 4)) {
        lastPlayerWhoKickedTheBall = 0;
    }
    if (runningSpeed > 0) {
        if (lastPlayerWhoKickedTheBall == player.render) {
            // lock direction until contact
            if (!touchBall) {
                dir = glm::normalize(TRANSFORM(ball)->position - TRANSFORM(player.hitzone)->position);
            }
        }

        if (dir.x || dir.y) {
            TRANSFORM(player.hitzone)->position += runningSpeed * dt * glm::normalize(dir) * glm::min(dirLength, 1.0f);
            LOGV(1, __(TRANSFORM(player.hitzone)->position) << __(dir) << __(dirLength) << __(player.previousDir));
        }

        if (dir.x < 0) {
            RENDERING(player.render)->flags |= RenderingFlags::MirrorHorizontal;
        } else {
            RENDERING(player.render)->flags &= ~RenderingFlags::MirrorHorizontal;
        }
    }

    player.previousDir = dir;

    if (game->gameThreadContext->joystickAPI->hasClicked(player.joystick, 1) ||
        (mouseControlAllowed && theTouchInputManager.hasClicked(2))) {
        // change active player
        float minDistToTheBall = 100000000;
        int idx = 0;
        for (int i = 0; i < players.size(); i++) {
            const auto& p = players[i];
            if (p.render == player.render) {
                continue;
            }
            if (p.joystick >= 0) {
                continue;
            }
            float dist = glm::distance(TRANSFORM(p.hitzone)->position, TRANSFORM(ball)->position);
            if (dist < minDistToTheBall) {
                minDistToTheBall = dist;
                idx = i;
            }
        }
        players[idx].joystick = player.joystick;
        player.joystick = -1;

        return false;
    }

    return true;
}

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);

    for (auto& player : players) {
        if (player.joystick >= 0) {
            if (!updateControlledPlayer(player, dt, this)) {
                // silly hack to prevent continous player switching
                break;
            }
        } else {
            ANIMATION(player.render)->name = HASH("idle", 0xed137eaa);
        }
    }

    // adjust camera position
    glm::vec2 diff = TRANSFORM(ball)->position - TRANSFORM(camera)->position;
    float l = glm::length(diff);
    TRANSFORM(camera)->position += diff * glm::min(1.5f, l) * dt;

    {
        Interval<float> cameraInterval;
        cameraInterval.t1 = TRANSFORM(camera)->position.y + TRANSFORM(camera)->size.y * 0.5f;
        cameraInterval.t2 = TRANSFORM(camera)->position.y - TRANSFORM(camera)->size.y * 0.5f;
        adjustZWithOnScreenPosition(this, ball, cameraInterval);
        for (auto plot : plots) {
            adjustZWithOnScreenPosition(this, plot, cameraInterval);
        }
        // adjustZWithOnScreenPosition(this, hitzone, cameraInterval);
    }

    // update ball angular velocity
    float ballSpeed = glm::length(PHYSICS(ball)->linearVelocity);
    PHYSICS(ball)->angularVelocity =
        glm::sign(PHYSICS(ball)->linearVelocity.x) * ballSpeed * tuning.f(HASH("ball_angular_vel", 0xbe84c99b));

    // bounce
    if (COLLISION(ballHitzone)->collision.count > 0) {
        // bounce as if 2 objects are spheres
        glm::vec2 toCollider = glm::normalize(TRANSFORM(COLLISION(ballHitzone)->collision.with[0])->position -
                                              TRANSFORM(ballHitzone)->position);

        glm::vec2 velocity = PHYSICS(ball)->linearVelocity;

        float proj = glm::dot(velocity, toCollider);

        if (proj > 0) {
            glm::vec2 newVelocity =
                velocity + (-2 * proj * tuning.f(HASH("ball_bounce_factor", 0x9d5d189d))) * toCollider;

            PHYSICS(ball)->linearVelocity = newVelocity;
            LOGI("Bounce");
            lastPlayerWhoKickedTheBall = 0;
        }
    }
}
