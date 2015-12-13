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

#include "base/SceneState.h"
#include "util/Random.h"
#include "PrototypeGame.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "base/EntityManager.h"
#include "PlayerSystem.h"

struct PlaceYourBetsScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;

    PlaceYourBetsScene(PrototypeGame* game)
        : SceneState<Scene::Enum>(
              "PlaceYourBets", SceneEntityMode::DoNothing, SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void setup(AssetAPI*) override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    int firstPlayer;
    int currentPlayer;
     const int CoinCount = 8;
    void onEnter(Scene::Enum) override {
         const int PlayerCount = 4;
        int pos[PlayerCount * 2 + CoinCount * 2];
        // generate coords for players and coins
        // they must be different
    generate:
        Random::N_Ints(sizeof(pos)/sizeof(int), pos, 0, MAZE_SIZE - 1);
        for (int i=0; i<PlayerCount+CoinCount; i++) {
            for (int j=i+1; j<PlayerCount+CoinCount; j++) {
                if (pos[2*i] == pos[2*j] &&
                    pos[2*i+1] == pos[2*j+1]) {
                    goto generate;
                }
            }
        }

        for (int i=0; i<4; i++) {
            TRANSFORM(game->guy[i])->position =
                TRANSFORM(game->grid[pos[2*i]][pos[2*i+1]].e)->position;
            RENDERING(game->guy[i])->show = false;
            TEXT(game->ui[i].bet)->show = false;
            PLAYER(game->guy[i])->score[game->round].bet = 0;
            PLAYER(game->guy[i])->score[game->round].coinCount = 0;
            PLAYER(game->guy[i])->score[game->round].score = 0;
        }

        for (int i=0; i<CoinCount; i++) {
            Entity coin = theEntityManager.CreateEntityFromTemplate("coin");
            TRANSFORM(coin)->position =
                TRANSFORM(game->grid[pos[PlayerCount * 2 + 2*i]][pos[PlayerCount * 2 + 2*i+1]].e)->position;
            game->coins.push_back(coin);
        }

        firstPlayer = currentPlayer = Random::Int(0, 3);
        RENDERING(game->guy[currentPlayer])->show = true;
        TEXT(game->ui[currentPlayer].bet)->show = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        char tmp[64];

        {
            int& bet = PLAYER(game->guy[currentPlayer])->score[game->round].bet;
            if (bet < 8 && PLAYER(game->guy[currentPlayer])->input.directions[direction::N] == InputState::Released) {
                PLAYER(game->guy[currentPlayer])->score[game->round].bet++;
            } else if (bet > 0 && PLAYER(game->guy[currentPlayer])->input.directions[direction::S] == InputState::Released) {
                PLAYER(game->guy[currentPlayer])->score[game->round].bet--;
            }

            sprintf(tmp, "%d - 8", PLAYER(game->guy[currentPlayer])->score[game->round].bet);
            TEXT(game->ui[currentPlayer].bet)->show = true;
            RENDERING(game->guy[currentPlayer])->show = true;
            TEXT(game->ui[currentPlayer].bet)->text = tmp;
        }

        int sum = 0;
        for (int i=0; i<4; i++) {
            sum += PLAYER(game->guy[i])->score[game->round].bet;
        }
        bool isLastPlayer = ((currentPlayer + 1) % 4 == firstPlayer);
        bool isSumValid = CoinCount < sum;

        if (isLastPlayer && !isSumValid) {
            TEXT(game->ui[currentPlayer].bet)->color = Color(0, 0, 0);
        } else {
            TEXT(game->ui[currentPlayer].bet)->color = Color(1, 1, 1);

            if (PLAYER(game->guy[currentPlayer])->input.actions[0] == InputState::Released) {
                RENDERING(game->guy[currentPlayer])->show = false;
                currentPlayer = (currentPlayer + 1) % 4;
                if (currentPlayer == firstPlayer) {
                    return Scene::InGame;
                } else {
                    RENDERING(game->guy[currentPlayer])->show = true;
                }
            }
        }

        return Scene::PlaceYourBets;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            RENDERING(game->guy[i])->show = true;
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreatePlaceYourBetsSceneHandler(PrototypeGame* game) {
        return new PlaceYourBetsScene(game);
    }
}
