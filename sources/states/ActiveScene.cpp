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
#include "systems/ButtonSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "WeaponSystem.h"
#include "SoldierSystem.h"
#include "MessageSystem.h"

#include <glm/gtx/norm.hpp>
#include "PrototypeGame.h"
#include "steering/SteeringBehavior.h"

#include "api/linux/NetworkAPILinuxImpl.h"
#include "api/KeyboardInputHandlerAPI.h"

struct ActiveScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity selected, waypoint;
    glm::vec2 speed, target;
    float accum;

    int latestSelectEntityKbEvents;

    ActiveScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
      this->game = game;
    }

    void setup() {
        selected = 0;
        waypoint = theEntityManager.CreateEntityFromTemplate("waypoint");

        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyRelease(10, [this] () -> void {
                latestSelectEntityKbEvents = 0;
        });
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyRelease(11, [this] () -> void {
                latestSelectEntityKbEvents = 1;
        });
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyRelease(12, [this] () -> void {
                latestSelectEntityKbEvents = 2;
        });
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
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        if (game->isGameHost)
            theWeaponSystem.Update(dt);

        float duration = 5;
        game->config.get("", "active_duration", &duration);
        accum = glm::min(duration, accum + dt);

        std::stringstream ss;
        ss.precision(1);
        ss << "GO " << accum << " s";
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
                Entity msg = theEntityManager.CreateEntityFromTemplate("message");
                MESSAGE(msg)->type = Message::ChangeState;
                MESSAGE(msg)->newState = Scene::Paused;
                return Scene::Paused;
            }
        }

        for (auto p: theMessageSystem.getAllComponents()) {
            switch (p.second->type) {
                case Message::ChangeState:
                    return p.second->newState;
                case Message::NewHealth:
                    SOLDIER(p.second->soldier)->health = p.second->health;
                    break;
            }
        }
        if (game->cameraMoveManager.update(dt))
            return Scene::Active;

        for (unsigned i=0; i<game->players.size(); i++) {
            auto p = game->players[i];
            if (BUTTON(p)->clicked || latestSelectEntityKbEvents == i) {
                selected = p;
                speed = glm::vec2(0.0f);
                target = TRANSFORM(selected)->position;

                if (latestSelectEntityKbEvents == i) {
                    game->cameraMoveManager.centerOn(target);
                    latestSelectEntityKbEvents = -1;
                }
                break;
            }
        }

        if (selected) {
            // MOVE
            if (theTouchInputManager.hasClicked()) {
                const auto& p = theTouchInputManager.getTouchLastPosition();
                if (glm::abs(p.x) <= 10 && glm::abs(p.y) <= 10) {
                    TRANSFORM(waypoint)->position = target = p;
                }
            }
            auto* tc = TRANSFORM(selected);

            if (glm::distance2(tc->position, target) > 0.0001) {
                glm::vec2 accel = SteeringBehavior::arrive(
                        tc->position, speed, target , 5, 0.1);
                speed += accel;

                //glm::vec2 newPosition = tc->position + speed * dt;

                tc->position += speed * dt;
                if (speed.x != 0)
                    tc->rotation = glm::atan2(speed.y, speed.x);
                else
                    tc->rotation = 0;
            } else {
                speed = glm::vec2(0.0f);
            }

            // SHOT
            if (SOLDIER(selected)->weapon) {
                WEAPON(SOLDIER(selected)->weapon)->fire = theTouchInputManager.isTouched(1);

                if (theTouchInputManager.isTouched(1)) {
                    const glm::vec2 shoot = theTouchInputManager.getTouchLastPosition(1) - TRANSFORM(selected)->position;
                    tc->rotation = glm::atan2(shoot.y, shoot.x);
                }
            }
        }

        return Scene::Active;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {

    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateActiveSceneHandler(PrototypeGame* game) {
        return new ActiveScene(game);
    }
}
