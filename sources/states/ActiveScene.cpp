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
#include "systems/TransformationSystem.h"
#include "WeaponSystem.h"
#include "SoldierSystem.h"

#include <glm/gtx/norm.hpp>
#include "PrototypeGame.h"
#include "steering/SteeringBehavior.h"

#include "api/linux/NetworkAPILinuxImpl.h"

struct ActiveScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity selected, waypoint;
    glm::vec2 speed, target;

    ActiveScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
      this->game = game;
    }

    void setup() {
        selected = 0;
        waypoint = theEntityManager.CreateEntityFromTemplate("waypoint");
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onPreEnter(Scene::Enum) override {

    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        if (game->cameraMoveManager.update(dt))
            return Scene::Active;

        for (Entity p: game->players) {
            if (BUTTON(p)->clicked) {
                selected = p;
                speed = glm::vec2(0.0f);
                target = TRANSFORM(selected)->position;
                break;
            }
        }

        if (selected) {
            // MOVE
            if (theTouchInputManager.hasClicked()) {
                TRANSFORM(waypoint)->position =
                    target = theTouchInputManager.getTouchLastPosition();
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
        
        auto* n  = static_cast<NetworkAPILinuxImpl*>(game->gameThreadContext->networkAPI);
        if (!n || n->getStatus() == NetworkStatus::InRoomAsMaster)
            theWeaponSystem.Update(dt);

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
