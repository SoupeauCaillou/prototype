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

#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"

#include "PrototypeGame.h"

struct GameEndScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    Entity wonText;
    Entity goToMenuBtn, restartBtn;

    GameEndScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        wonText = theEntityManager.CreateEntity("text_won",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("text"));

        goToMenuBtn =  theEntityManager.CreateEntity("button_gotomenu",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("button_gotomenu"));

        restartBtn = theEntityManager.CreateEntity("button_restart",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("button_restart"));
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        for (int cell = 0; cell < 81; ++cell) {
            RENDERING(game->grid[cell])->show =
            BUTTON(game->grid[cell])->enabled = false;
        }
        TEXT(wonText)->text = "Player ??todo?? has won the match!";

        TEXT(wonText)->show =
        RENDERING(goToMenuBtn)->show = BUTTON(goToMenuBtn)->enabled =
        RENDERING(restartBtn)->show = BUTTON(restartBtn)->enabled = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(restartBtn)->clicked) {
            return Scene::GameStart;
        } else if (BUTTON(goToMenuBtn)->clicked) {
            return Scene::Menu;
        }
        return Scene::GameEnd;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
        TEXT(wonText)->show =
        RENDERING(goToMenuBtn)->show = BUTTON(goToMenuBtn)->enabled =
        RENDERING(restartBtn)->show = BUTTON(restartBtn)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameEndSceneHandler(PrototypeGame* game) {
        return new GameEndScene(game);
    }
}
