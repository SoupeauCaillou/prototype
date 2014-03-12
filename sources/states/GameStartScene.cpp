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

#include "base/TouchInputManager.h"
#include "base/EntityManager.h"

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "PrototypeGame.h"

struct GameStartScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    GameStartScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {

        //create sheeps
        int count = 1;
        for (int i = 0; i < count; i++) {
            Entity e = theEntityManager.CreateEntityFromTemplate("game/sheep");
            TRANSFORM(e)->position.x += i;
        }

        auto sheep = theAutonomousAgentSystem.RetrieveAllEntityWithComponent();

        // not very well optimized...
        for (auto s : sheep) {
            for (auto s2 : sheep) {
                if (s2 != s) {
                    AUTONOMOUS(s)->obstacles.push_back(s2);
                }
            }
        }


        //create obstacles
        for (int i = 0; i < 4; i++) {
            Entity e = theEntityManager.CreateEntityFromTemplate("game/wall");
            TRANSFORM(e)->position = glm::rotate(TRANSFORM(e)->position, glm::radians(90.f * i));
            TRANSFORM(e)->rotation = glm::radians(90.f * i);

            for (auto s : sheep) {
                AUTONOMOUS(s)->obstacles.push_back(e);
            }

        }
        Entity buisson = theEntityManager.CreateEntityFromTemplate("game/buisson");
        TRANSFORM(buisson)->position = glm::vec2(0.f);
        TRANSFORM(buisson)->size = glm::vec2(1.f);
        for (auto s : sheep) {
            AUTONOMOUS(s)->obstacles.push_back(buisson);
        }

    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        return Scene::InGame;
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
    StateHandler<Scene::Enum>* CreateGameStartSceneHandler(PrototypeGame* game) {
        return new GameStartScene(game);
    }
}
