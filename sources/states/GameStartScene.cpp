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
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"

#include "systems/PlayerSystem.h"
#include "../PrototypeGame.h"

struct GameStartScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity getReady;

    GameStartScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() { }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        getReady = theEntityManager.CreateEntityFromTemplate("menu/ready_screen");
        PLAYER(game->player)->ready = false;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        if (theTouchInputManager.hasClicked()) {
            PLAYER(game->player)->ready = !PLAYER(game->player)->ready;
        }

        if (PLAYER(game->player)->ready) {
            int readyCount = 0;
            thePlayerSystem.forEachECDo([&readyCount] (Entity e, PlayerComponent* pc) -> void {
                readyCount += pc->ready;
            });
            
            if (readyCount == thePlayerSystem.entityCount()) {
                TEXT(getReady)->text = "Everyone is ready";
                RENDERING(getReady)->color = Color(0, 1, 0, 0.5);
                return Scene::InGame;
            } else {
                std::stringstream a;
                a << readyCount << " - " << thePlayerSystem.entityCount() << " players ready";
                TEXT(getReady)->text = a.str();
                RENDERING(getReady)->color = Color(1, 0.5, 0, 0.5);
            }
        } else {
            TEXT(getReady)->text = "Click When Ready";
            RENDERING(getReady)->color = Color(1, 0, 0, 0.5);
        }

        return Scene::GameStart;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    float accum;
    void onPreExit(Scene::Enum) override {
        accum = 1;
    }

    bool updatePreExit(Scene::Enum, float dt) override{
        return (accum -= dt) < 0;
    }

    void onExit(Scene::Enum) override {
        theEntityManager.DeleteEntity(getReady);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameStartSceneHandler(PrototypeGame* game) {
        return new GameStartScene(game);
    }
}
