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
#include "systems/MorpionGridSystem.h"

#include "PrototypeGame.h"

struct TurnEndScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    TurnEndScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        int i = MORPION_GRID(game->lastPlayedCell)->i;
        int j = MORPION_GRID(game->lastPlayedCell)->j;
        if (theMorpionGridSystem.isMiniMorpionFinished(i, j)) {
            for (auto e : theMorpionGridSystem.getCellsForMiniMorpion(i, j, MorpionGridComponent::Available)) {
                MORPION_GRID(e)->type = MorpionGridComponent::Lost;
            }
        }
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        //check if maxi morpion is finished too
        if (theMorpionGridSystem.isMaxiMorpionFinished()) {
            return Scene::GameEnd;
        }

        return Scene::TurnStart;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        game->currentPlayer = (game->currentPlayer == game->player1) ?
            game->player2
            : game->player1;
    }

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateTurnEndSceneHandler(PrototypeGame* game) {
        return new TurnEndScene(game);
    }
}
