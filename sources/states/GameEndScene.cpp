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
#include "systems/AnchorSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "util/Random.h"
#include "PrototypeGame.h"

#include "api/LocalizeAPI.h"

struct GameEndScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    Entity done[4];

    GameEndScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        for (int i=0; i<4; i++) {
            done[i] = theEntityManager.CreateEntityFromTemplate("menu/player_done_text");
            ANCHOR(done[i])->parent = game->playerButtons[i];

            if (i % 2) {
                ANCHOR(done[i])->anchor = -ANCHOR(done[i])->anchor;
                TEXT(done[i])->positioning = 1 - TEXT(done[i])->positioning;
            }
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    Entity selectedBee;

    std::map<Entity, int> bee2player;
    std::map<Entity, Entity> highlight;
    int playerReadyCount;

    void onEnter(Scene::Enum) override {
        playerReadyCount = 0;
        for (int i=0; i<4; i++) {
            if (game->playerActive[i] >= 0) {
                BUTTON(game->playerButtons[i])->enabled =
                    RENDERING(game->playerButtons[i])->show = true;
                RENDERING(game->playerButtons[i])->color.a = 0.2;
                TEXT(done[i])->show = true;
            } else {
                playerReadyCount++;
            }
        }

        for (auto b: game->bees) {
            BUTTON(b)->enabled = true;
            PHYSICS(b)->mass = 0;
        }
        selectedBee = 0;
    }

    int playerSelectedCount(int i) {
        int count = 0;
        for (auto sel: bee2player) {
            if (sel.second == i) count++;
        }
        return count;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        for (auto b: game->bees) {
            if (BUTTON(b)->clicked) {
                if (selectedBee) {
                    theEntityManager.DeleteEntity(highlight[selectedBee]);
                    highlight.erase(selectedBee);
                }

                auto it = bee2player.find(b);
                Entity h = 0;

                selectedBee = b;
                if (it == bee2player.end()) {
                    h = theEntityManager.CreateEntityFromTemplate("game/bee_highlight");
                    ANCHOR(h)->parent = b;
                    TRANSFORM(h)->size = TRANSFORM(ANCHOR(h)->parent)->size * 2.0f;
                    highlight[b] = h;
                } else {
                    bee2player.erase(it);
                    h = highlight[b];
                }
                RENDERING(h)->color = game->playerColors[0];

                return Scene::GameEnd;
            }
        }

        if (selectedBee) {
            for (int i = 0; i<4; i++) {
                int count = playerSelectedCount(i);

                if (count < (game->playerActive[i] + 1)) {
                    if (BUTTON(game->playerButtons[i])->clicked) {
                        bee2player[selectedBee] = i;
                        RENDERING(highlight[selectedBee])->color = game->playerColors[1 + i];
                        selectedBee = 0;

                        return Scene::GameEnd;
                    }
                }
            }
        }

        for (int i=0; i<4; i++) {
            int count = playerSelectedCount(i);

            char* scoreText = (char*) alloca(50);
            sprintf(scoreText, game->gameThreadContext->localizeAPI->text("lookup").c_str(), count, game->playerActive[i] + 1);
            TEXT(done[i])->text = scoreText;

            if (count == (game->playerActive[i] + 1)) {
                TEXT(done[i])->text = game->gameThreadContext->localizeAPI->text("done");
                if (BUTTON(game->playerButtons[i])->clicked) {
                    BUTTON(game->playerButtons[i])->enabled = false;
                    playerReadyCount++;
                }
            }
        }

        if (playerReadyCount == 4)
            return Scene::GameStart;

        return Scene::GameEnd;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            BUTTON(game->playerButtons[i])->enabled =
                RENDERING(game->playerButtons[i])->show = false;
            RENDERING(game->playerButtons[i])->color.a = 1.0;
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameEndSceneHandler(PrototypeGame* game) {
        return new GameEndScene(game);
    }
}
