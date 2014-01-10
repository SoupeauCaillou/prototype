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

#include "base/StateMachine.h"
#include "Scenes.h"

#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/JoystickManager.h"
#include "systems/ButtonSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "WeaponSystem.h"
#include "SoldierSystem.h"
#include "MessageSystem.h"
#include "FlagSystem.h"
#include "TeamSystem.h"
#include "SelectionSystem.h"

#include <glm/gtx/norm.hpp>
#include "PrototypeGame.h"
#include "steering/SteeringBehavior.h"

#include "api/linux/NetworkAPILinuxImpl.h"
#include "api/KeyboardInputHandlerAPI.h"

struct ActiveScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity waypoint;
    glm::vec2 speed, target;
    bool targetSet;
    float accum;
    Entity restart;

    int latestSelectEntityKbEvents;

    ActiveScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
      this->game = game;
      restart = 0;
    }

    void setup() {
        waypoint = theEntityManager.CreateEntityFromTemplate("waypoint");

        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(25, [this] () -> void {
                // up
                game->cameraMoveManager.addSpeed(glm::vec2(0.0f, 1.0f));
        });
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(39, [this] () -> void {
                // down
                game->cameraMoveManager.addSpeed(glm::vec2(0.0f, -1.0f));
        });
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(38, [this] () -> void {
                // left
                game->cameraMoveManager.addSpeed(glm::vec2(-1.0f, 0.0f));
        });
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(40, [this] () -> void {
                // right
                game->cameraMoveManager.addSpeed(glm::vec2(1.0f, 0.0f));
        });
        
        restart = theEntityManager.CreateEntityFromTemplate("restart");
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onPreEnter(Scene::Enum) override {

    }

    void onEnter(Scene::Enum) override {
        accum = 0;
        for (Entity p: game->players) {
            COLLISION(p)->restorePositionOnCollision = true;
        }
        latestSelectEntityKbEvents = -1;
        targetSet = false;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        if (game->isGameHost && BUTTON(restart)->clicked) {
            return Scene::GameStart;
        }

        for (Entity p: game->players) {
            SELECTION(p)->enabled = (SOLDIER(p)->health > 0);
        }

        // RENDERING(selection)->show = RENDERING(waypoint)->show = (selected != 0);
        // ANCHOR(selection)->rotation += dt * 3;

        if (game->isGameHost) {
            theWeaponSystem.Update(dt);
            theFlagSystem.Update(dt);

            // check point condition
            for (auto& pteam: theTeamSystem.getAllComponents()) {
                auto* tc = pteam.second;
                if (tc->flagCaptured) {
                    tc->score++;
                    tc->flagCaptured = false;
                    std::stringstream ss;
                    ss << tc->score;
                    TEXT(tc->spawn)->text = ss.str();
                    return Scene::GameStart;
                }
            }
        }

        //------------- TIMER
        //
        //
        float duration = 5;
        game->config.get("", "active_duration", &duration);
        accum = glm::min(duration, accum + dt);

        std::stringstream ss;
        ss << "GO " << std::ceil(accum) << " s";
        TRANSFORM(game->timer)->size = TRANSFORM(game->camera)->size;
        TRANSFORM(game->timer)->size *= glm::vec2(1.0f - accum / duration, 0.05);
        TEXT(game->timer)->text = ss.str();
        TEXT(game->timer)->charHeight = TRANSFORM(game->timer)->size.y;
        TRANSFORM(game->timer)->position =
            AnchorSystem::adjustPositionWithCardinal(TRANSFORM(game->camera)->position + TRANSFORM(game->camera)->size * glm::vec2(0.0f, 0.5f),
            TRANSFORM(game->timer)->size,
            Cardinal::N);
        if (game->isGameHost) {
            if (accum >= duration) {
                return Scene::Paused;
            }
        }
        //-------------------

        //--------- Messaging
        for (auto p: theMessageSystem.getAllComponents()) {
            switch (p.second->type) {
                case Message::ChangeState:
                    return p.second->newState;
                case Message::NewHealth:
                    SOLDIER(p.second->soldier)->health = p.second->health;
                    break;
                default:
                    break;
            }
        }
        //-------------------

        //-------------------

        //--------- Selection
        theSelectionSystem.Update(dt);
        //-------------------

        //--- Soldiers action
        bool someoneSelected = false;
        for (auto p: game->players) {
            const auto* selc = SELECTION(p);
            WEAPON(SOLDIER(p)->weapon)->fire = false;
            if (!selc->enabled || !selc->selected) {
                continue;
            }
            someoneSelected = true;
            game->cameraMoveManager.setZoom(3.0f);

            if (selc->newlySelected) {
                targetSet = false;
                break;
            } else {
                // MOVE
                if (theTouchInputManager.hasClicked()) {
                    const auto& pos = theTouchInputManager.getTouchLastPosition();
                    if (glm::abs(pos.x) <= theCollisionSystem.worldSize.x && glm::abs(pos.y) <= theCollisionSystem.worldSize.y) {
                        TRANSFORM(waypoint)->position = target = pos;
                        targetSet = true;
                    }
                } else {
                    glm::vec2 pos = theJoystickManager.getPadDirection(0, JoystickPad::LEFT);
                    if (glm::length2(pos) > 0.01f) {
                        pos += TRANSFORM(p)->position;

                        TRANSFORM(waypoint)->position = target = pos;
                        targetSet = true;                        
                    }
                }
                auto* tc = TRANSFORM(p);

                if (targetSet && glm::distance2(tc->position, target) > 0.0001) {
                    glm::vec2 accel = SteeringBehavior::arrive(
                            tc->position, speed, target , 5, 0.1);
                    speed += accel;

                    tc->position += speed * dt;
                    if (speed.x != 0)
                        tc->rotation = glm::atan2(speed.y, speed.x);
                    else
                        tc->rotation = 0;
                } else {
                    speed = glm::vec2(0.0f);
                }

                // SHOT
                if (SOLDIER(p)->weapon) {                    
                    glm::vec2 shoot;
                    if (theTouchInputManager.isTouched(1)) {
                        shoot = theTouchInputManager.getTouchLastPosition(1) - TRANSFORM(p)->position;
                        WEAPON(SOLDIER(p)->weapon)->fire = true;
                    } else if (glm::length2(theJoystickManager.getPadDirection(0, JoystickPad::RIGHT)) > 0.01) {
                        shoot = theJoystickManager.getPadDirection(0, JoystickPad::RIGHT);
                        WEAPON(SOLDIER(p)->weapon)->fire = true;
                    }

                    if (WEAPON(SOLDIER(p)->weapon)->fire) {
                        tc->rotation = glm::atan2(shoot.y, shoot.x);
                    }
                }
            }
            TRANSFORM(game->camera)->position = TRANSFORM(p)->position;
        }
        //-------------------

        //------------ Camera
        if (!someoneSelected) {
            /// game->cameraMoveManager.setZoom(1.0f);

            if (game->cameraMoveManager.update(dt))
                return Scene::Active;
        }
        //-------------------

        return Scene::Active;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum to) override {
        if (game->isGameHost) {
            Entity msg = theEntityManager.CreateEntityFromTemplate("message");
            MESSAGE(msg)->type = Message::ChangeState;
            MESSAGE(msg)->newState = to;
        }

        for (Entity p: game->players) {
            WEAPON(SOLDIER(p)->weapon)->fire = false;
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateActiveSceneHandler(PrototypeGame* game) {
        return new ActiveScene(game);
    }
}
