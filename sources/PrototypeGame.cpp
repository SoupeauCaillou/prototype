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

#include "FootieSystem.h"
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

#include <SDL2/SDL_keycode.h>

#include <algorithm>

PrototypeGame::PrototypeGame() : Game() { registerScenes(this, sceneStateMachine); }



struct Player {
    Player() {
        joystick = -1;
    }

    Entity entity;
    int joystick;
};
std::vector<Player> players;

Entity ballHitzone;
Entity terrain;
std::vector<Entity> plots;
std::vector<Entity> hitzones;


void addHitzone(PrototypeGame* game, Entity e) {
    Entity h = theEntityManager.CreateEntityFromTemplate("hitzone");
    ANCHOR(h)->parent = e;
    TRANSFORM(h)->size = glm::vec2(TRANSFORM(e)->size.x);

    hitzones.push_back(h);

    if (e == game->ball) {
        COLLISION(h)->group = 2;
        COLLISION(h)->collideWith = 0xffffffff;
        COLLISION(h)->restorePositionOnCollision = true;
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
    FootieSystem::CreateInstance();

    FootieSystem::GetInstancePointer()->game = this;

    orderedSystemsToUpdate.push_back(FootieSystem::GetInstancePointer());
    orderedSystemsToUpdate.push_back(ShadowSystem::GetInstancePointer());

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    {
        std::string colors[] = { "color", "white", "color", "orange_blue" };
        const char* animations[] = { "idle", "run", "tackle", "walk" };
        for (int i=0; i<2; i++) {
            for  (int j=0; j<4; j++) {
                char animationName[64];
                sprintf(animationName, "%s_%s", animations[j], colors[2*i + 1].c_str());
                theAnimationSystem.loadAnim(gameThreadContext->assetAPI, animationName, animations[j], &colors[2 * i], 1);

                teamAnimations[i][(actions::Enum) j] = Murmur::RuntimeHash(animationName);
            }
        }
    }

    CAMERA(camera)->clearColor = Color(0.1, 0.55, 0.05);

    theCollisionSystem.worldSize = glm::vec2(40, 20);

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
                p.entity = theEntityManager.CreateEntityFromTemplate("player");
                Entity hitzone = FOOTIE(p.entity)->hitzone = theEntityManager.CreateEntityFromTemplate("player_hitzone");
                TRANSFORM(hitzone)->position = position;
                ANCHOR(p.entity)->parent = hitzone;
                addShadow(this, p.entity);
                FOOTIE(p.entity)->team = i % 2;
                players.push_back(p);

                all.push_back(p.entity);
            } else if (entity == "cage") {
                Entity e = theEntityManager.CreateEntityFromTemplate(entity.c_str());
                TRANSFORM(e)->position = position;
                Entity p1 = theEntityManager.CreateEntityFromTemplate("poteau");
                ANCHOR(p1)->parent = e;
                Entity p2 = theEntityManager.CreateEntityFromTemplate("poteau");
                ANCHOR(p2)->parent = e;
                ANCHOR(p2)->position.y +=
                    tuning.f(HASH("poteau_diff_y_haut_bas", 0x7218439b));
                Entity back = theEntityManager.CreateEntityFromTemplate("dos_cage");
                ANCHOR(back)->parent = e;

                int mirror = 0;
                dfp.get(section, HASH("mirror", 0xcf083e19), &mirror);
                if (mirror) {
                    ANCHOR(back)->position.x = -ANCHOR(back)->position.x;
                    RENDERING(e)->flags |= RenderingFlags::MirrorHorizontal;
                }

                hitzones.push_back(p1);
                hitzones.push_back(p2);
                hitzones.push_back(back);
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

    terrain = theEntityManager.CreateEntityFromTemplate("terrain");
    TRANSFORM(terrain)->z += 0.01;
    Entity border = theEntityManager.CreateEntityFromTemplate("terrain");
    TRANSFORM(border)->size.x += 0.25;
    TRANSFORM(border)->size.y += 0.25;
    RENDERING(border)->color = Color(1, 1, 1);

    sceneStateMachine.start(Scene::Menu);

    float x_pos[100], y_pos[100];
    glm::vec2 hSize = TRANSFORM(terrain)->size * 0.5f;
    Random::N_Floats(100, x_pos, -hSize.x, hSize.x);
    Random::N_Floats(100, y_pos, -hSize.y, hSize.y);
    for (int i = 0; i < 100; i++) {
        Entity b = theEntityManager.CreateEntityFromTemplate("brush");
        TRANSFORM(b)->position = glm::vec2(x_pos[i], y_pos[i]);
    }
}

static void adjustZWithOnScreenPosition(Game* game, Entity e, const Interval<float>& camera) {
    auto* tc = TRANSFORM(e);
    float t = camera.invLerp(tc->position.y);
    Interval<float> z(game->tuning.f(HASH("adjust_min_z", 0x533b118a)),
                      game->tuning.f(HASH("adjust_max_z", 0x583d04f9)));
    tc->z = z.lerp(t);
}

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);

    // input handling
    for (auto& player : players) {
        const int mapping[buttons::Count] = { 's', 'd', 'q', SDLK_LSHIFT, 'e' };

        if (player.joystick >= 0) {
            FootieComponent* f = FOOTIE(player.entity);
            bool kbAllowed = player.joystick == 0;

            /* buttons */
            for (int i=0; i<buttons::Count; i++) {
                if (gameThreadContext->joystickAPI->isDown(player.joystick, i) ||
                    (kbAllowed &&
                     gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(mapping[i]))) {
                    f->input.accums[i] += dt;
                } else {
                    f->input.released[i] = (f->input.accums[i] > 0);
                    f->input.accums[i] = 0;
                }
            }

            f->input.direction = gameThreadContext->joystickAPI->getPadDirection(player.joystick, 0);
            float l = glm::length(f->input.direction);
            if (l > 1) {
                f->input.direction /= l;
            }
            if (l <= 0 && kbAllowed) {
                if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_LEFT)) {
                    f->input.direction.x = -1;
                } else if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_RIGHT)) {
                    f->input.direction.x = 1;
                }
                if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_UP)) {
                    f->input.direction.y = 1;
                } else if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_DOWN)) {
                    f->input.direction.y = -1;
                }
                l = glm::length(f->input.direction);
                if (l > 0) {
                    f->input.direction /= l;
                }
            }
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

    // field limits
    if (!IntersectionUtil::rectangleRectangle(TRANSFORM(ball), TRANSFORM(terrain))) {
        TRANSFORM(ball)->position =
            PHYSICS(ball)->linearVelocity = glm::vec2(0.0f);
        PHYSICS(ball)->forces.clear();
        lastPlayerWhoKickedTheBall = 0;
    }

    // update ball angular velocity
    float ballSpeed = glm::length(PHYSICS(ball)->linearVelocity);
    PHYSICS(ball)->angularVelocity =
        glm::sign(PHYSICS(ball)->linearVelocity.x) * ballSpeed * tuning.f(HASH("ball_angular_vel", 0xbe84c99b));

    // bounce
    if (COLLISION(ballHitzone)->collision.count > 0) {
        glm::vec2 normalAwayFromCollider(0.0f);

        if (std::find(
            plots.begin(),
            plots.end(),
            COLLISION(ballHitzone)->collision.with[0]) != plots.end()) {
            // bounce as if 2 objects are spheres
            normalAwayFromCollider = -glm::normalize(TRANSFORM(COLLISION(ballHitzone)->collision.with[0])->position -
                                              TRANSFORM(ballHitzone)->position);
        } else {
            normalAwayFromCollider = COLLISION(ballHitzone)->collision.normal[0];
        }
        glm::vec2 velocity = PHYSICS(ball)->linearVelocity;

        float proj = glm::dot(velocity, normalAwayFromCollider);

        if (proj < 0) {
            glm::vec2 newVelocity =
                velocity +
                (2 * -proj * tuning.f(HASH("ball_bounce_factor", 0x9d5d189d)))
                * normalAwayFromCollider;

            PHYSICS(ball)->linearVelocity = newVelocity;
            LOGI("Bounce");
            lastPlayerWhoKickedTheBall = 0;
        }
    }
}
