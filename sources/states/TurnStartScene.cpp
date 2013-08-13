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
#include "systems/PlayerSystem.h"
#include "systems/TicTacToeSystem.h"
#include "systems/ActionSystem.h"

#include "PrototypeGame.h"

struct TurnStartScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    std::vector<Entity> playableGridCells;

    TurnStartScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        auto * ttt = theTicTacToeSystem.getAllComponents().begin()->second;
        LOGI("Your turn player" << (ttt->currentPlayer == ttt->player1 ? "1" : "2"));

        playableGridCells = theMorpionGridSystem.nextPlayableCells(theTicTacToeSystem.getAllComponents()
            .begin()->second->lastPlayedCell);
        for (auto e : playableGridCells) {
            MORPION_GRID(e)->type = MorpionGridComponent::Playable;
        }
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        auto * ttt = theTicTacToeSystem.getAllComponents().begin()->second;
        for (auto e : playableGridCells) {
            //only the current player can select the grid!
            if (BUTTON(e)->clicked) {
                if (ttt->currentPlayer == thePlayerSystem.GetMyself(game->networkMode,
                game->gameThreadContext->networkAPI)) {
                    LOGE("It's not your turn to play!");
                } else {
                    Entity myPlayer = thePlayerSystem.GetMyself(game->networkMode, game->gameThreadContext->networkAPI);
                    ActionComponent* ac = ACTION(myPlayer);
                    ac->action = EAction::SelectedCell;
                    ac->SelectedCellParams.cell = e;
                    ac->SelectedCellParams.player = myPlayer;
                }
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
        for (auto e : playableGridCells) {
            if (e != theTicTacToeSystem.getAllComponents().begin()->second->lastPlayedCell) {
                MORPION_GRID(e)->type = MorpionGridComponent::Available;
            }
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateTurnStartSceneHandler(PrototypeGame* game) {
        return new TurnStartScene(game);
    }
}
