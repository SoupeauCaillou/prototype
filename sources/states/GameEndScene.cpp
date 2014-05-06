/*
    This file is part of Bzzz.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Bzzz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Bzzz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bzzz.  If not, see <http://www.gnu.org/licenses/>.
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
#include "BzzzGame.h"

#include "api/LocalizeAPI.h"

struct GameEndScene : public StateHandler<Scene::Enum> {
    BzzzGame* game;

    Entity done[4];
    bool ready[4];

    GameEndScene(BzzzGame* game) : StateHandler<Scene::Enum>() {
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

    void onEnter(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            if (game->playerActive[i] >= 0) {
                BUTTON(game->playerButtons[i])->enabled =
                    RENDERING(game->playerButtons[i])->show = true;
                RENDERING(game->playerButtons[i])->color.a = 0.2;
                TEXT(done[i])->show = true;
                ready[i] = false;
            } else {
                ready[i] = true;
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
                    BUTTON(selectedBee)->enabled = true;
                }

                auto it = bee2player.find(b);
                Entity h = 0;

                selectedBee = b;
                BUTTON(selectedBee)->enabled = false;

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
                ready[i] = true;
            }
        }

        bool everyoneReady = true;
        for (int i=0; i<4 && everyoneReady; i++) {
            everyoneReady &= ready[i];
        }
        if (everyoneReady)
            return Scene::GameStart;

        return Scene::GameEnd;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        LOGI("Score update");
        int idx = 0;
        for (int i=0; i<4; i++) {
            int total = 0;
            for (int j=0; j<game->playerActive[i] + 1; j++) {
                auto bee = game->selected[idx++];

                auto it = bee2player.find(bee);
                if (it != bee2player.end()) {
                    if (it->second == i) {
                        total++;
                    }
                }
            }
            if (total == (game->playerActive[i] + 1)) {
                game->score[i] += total;
            }
        }
    }

    void onExit(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            TEXT(done[i])->show = false;
            BUTTON(game->playerButtons[i])->enabled =
                RENDERING(game->playerButtons[i])->show = false;
            RENDERING(game->playerButtons[i])->color.a = 1.0;
        }

        for (auto h: highlight) {
            AUTO_DESTROY(h.second)->type = AutoDestroyComponent::LIFETIME;
            AUTO_DESTROY(h.second)->params.lifetime.freq.value = 0.5;
            ANCHOR(h.second)->parent = 0;
        }
        highlight.clear();
        bee2player.clear();

        for (auto b: game->bees) {
            AUTO_DESTROY(b)->type = AutoDestroyComponent::LIFETIME;
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameEndSceneHandler(BzzzGame* game) {
        return new GameEndScene(game);
    }
}
