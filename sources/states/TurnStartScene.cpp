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

#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"

#include "PrototypeGame.h"

struct TurnStartScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    TurnStartScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        LOGI("Your turn player" << (game->currentPlayer == game->player1 ? "1" : "2"));
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        for (int i = 0; i < 81; ++i) {
            if (BUTTON(game->grid[i])->clicked && RENDERING(game->grid[i])->color == Color(0., 0., 0.)) {
                RENDERING(game->grid[i])->color = (game->currentPlayer == game->player1 ?
                    Color(1., 0., 0.)
                    : Color(0., 1., 0.));

                return Scene::TurnEnd;
            }
        }

        return Scene::TurnStart;
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
    StateHandler<Scene::Enum>* CreateTurnStartSceneHandler(PrototypeGame* game) {
        return new TurnStartScene(game);
    }
}
