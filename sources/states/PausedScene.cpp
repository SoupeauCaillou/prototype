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

#include "SelectionSystem.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "systems/ButtonSystem.h"
#include "systems/SpotSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "MessageSystem.h"
#include "api/NetworkAPI.h"
#include "api/linux/NetworkAPILinuxImpl.h"

#include "PrototypeGame.h"
#include <iomanip>

struct PausedScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    float accum;

    PausedScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
      this->game = game;
    }

    void setup() {

    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        accum = 0;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        if (!game->cameraMoveManager.update(dt)) {
            //--------- Selection
            theSelectionSystem.Update(dt);
            //-------------------
        }
        
        float duration = 5;
        game->config.get("", "paused_duration", &duration);
        accum = glm::min(duration, accum + dt);

        std::stringstream ss;
        ss << "PAUSE " << std::ceil(accum) << " s";
        TRANSFORM(game->timer)->size = TRANSFORM(game->camera)->size;
        TRANSFORM(game->timer)->size *= glm::vec2(accum / duration, 0.05);
        TRANSFORM(game->timer)->position =
            AnchorSystem::adjustPositionWithCardinal(TRANSFORM(game->camera)->position + TRANSFORM(game->camera)->size * glm::vec2(0.0f, 0.5f),
            TRANSFORM(game->timer)->size,
            Cardinal::N);
        TEXT(game->timer)->text = ss.str();
        TEXT(game->timer)->charHeight = TRANSFORM(game->timer)->size.y;

        if (game->isGameHost) {
            if (accum >= duration) {
                Entity msg = theEntityManager.CreateEntityFromTemplate("message");
                MESSAGE(msg)->type = Message::ChangeState;
                MESSAGE(msg)->newState = Scene::Active;
                return Scene::Active;
            }
        } else {
            for (auto p: theMessageSystem.getAllComponents()) {
                if (p.second->type == Message::ChangeState)
                    return p.second->newState;
            }
        }

        return Scene::Paused;
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
    StateHandler<Scene::Enum>* CreatePausedSceneHandler(PrototypeGame* game) {
        return new PausedScene(game);
    }
}
