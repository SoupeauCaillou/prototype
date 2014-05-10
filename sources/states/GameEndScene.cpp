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
#include "systems/BlinkSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "util/Random.h"
#include "BzzzGame.h"

#include <glm/gtx/norm.hpp>
#include "api/LocalizeAPI.h"

struct GameEndScene : public StateHandler<Scene::Enum> {
    BzzzGame* game;

    Entity done[4];
    bool ready[4];
    float exitAccum;

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
    std::vector<Entity> blinks;

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
        exitAccum = 0;
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
                        BUTTON(selectedBee)->enabled = true;
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
            } else {
                ready[i] = false;
            }
        }

        bool everyoneReady = std::all_of(ready, ready + 4, [] (bool b) { return b; });

        if (everyoneReady) {
            LOGI("Score update");
            int idx = 0;
            bool victory = false;
            std::vector<std::pair<int, Entity>> failure;

            for (int i=0; i<4; i++) {
                int total = 0;
                for (int j=0; j<game->playerActive[i] + 1; j++) {
                    auto bee = game->selected[idx++];

                    bool correct = false;
                    auto it = bee2player.find(bee);
                    Entity hint = 0;
                    if (it != bee2player.end()) {
                        if (it->second == i) {
                            total++;
                            correct = true;
                        }
                    }

                    if (correct) {
                        hint = theEntityManager.CreateEntityFromTemplate("game/bee_found");
                        TEXT(hint)->charHeight = TRANSFORM(bee)->size.y * 1.5;
                    } else {
                        hint = theEntityManager.CreateEntityFromTemplate("game/bee_missing");
                    }
                    TRANSFORM(hint)->position = TRANSFORM(bee)->position;
                    blinks.push_back(hint);

                    if (correct) {
                        bee2player.erase(it);
                    } else {
                        failure.push_back(std::make_pair(i, bee));
                    }
                }

                if (total == (game->playerActive[i] + 1)) {
                    game->score[i] += total;

                    if (game->score[i] >= 10) {
                        victory = true;
                    }
                }
            }

            LOGI(failure.size() << " errors");
            float* blinkPeriods = new float[failure.size()];
            Random::N_Floats(failure.size(), blinkPeriods, 0, 1);
            int index = 0;
            for (auto pf: failure) {
                int pIndex = pf.first;
                Entity missedBee = pf.second;

                // Now find the nearest bee in the incorrectly
                // chosen by this player
                const auto& p = TRANSFORM(missedBee)->position;
                float dist = 0;
                Entity nearest = 0;

                for (auto b2p: bee2player) {
                    if (b2p.second == pIndex) {
                        float d = glm::distance2(TRANSFORM(b2p.first)->position, p);

                        if (nearest == 0 || d < dist) {
                            dist = d;
                            nearest = b2p.first;
                        }
                    }
                }

                LOGF_IF(nearest == 0, "Fix previous algo");
                // Make both blink using the same period
                auto blink = theEntityManager.CreateEntityFromTemplate("game/blink");
                BLINK(blink)->accum = blinkPeriods[index++];
                ANCHOR(blink)->parent = missedBee;
                TRANSFORM(blink)->size = TRANSFORM(missedBee)->size;
                RENDERING(blink)->color = game->playerColors[pIndex+1];
                blinks.push_back(blink);

                auto h = highlight[nearest];
                ADD_COMPONENT(h, Blink);
                *BLINK(h) = *(BLINK(blink));

                bee2player.erase(nearest);
            }


            return victory ? Scene::Victory : Scene::GameStart;
        }

        return Scene::GameEnd;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        for (int i=0; i<4;i ++) {
            if (TEXT(done[i])->show) {
                ready[i] = false;
                TEXT(done[i])->text = game->gameThreadContext->localizeAPI->text("ready");
            }
        }
    }

    bool updatePreExit(Scene::Enum, float dt) override {
        for (int i=0; i<4;i ++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                ready[i] = true;
                TEXT(done[i])->show = false;
            }
        }
        return std::all_of(ready, ready + 4, [] (bool b) { return b; });
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
        for (auto b: blinks) {
            theEntityManager.DeleteEntity(b);
        }
        blinks.clear();
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameEndSceneHandler(BzzzGame* game) {
        return new GameEndScene(game);
    }
}
