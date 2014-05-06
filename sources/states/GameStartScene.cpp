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

struct GameStartScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    struct PlayerText {
        union {
            struct {
                Entity score;
                Entity bet;
                Entity ready;
            };
            Entity t[3];
        };
    } texts[4];

    Entity playButton;
    bool ready[4];
    std::vector<Entity> highlights;

    std::list<Entity> walls;

    GameStartScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        for (int i=0; i<4; i++) {
            texts[i].score = theEntityManager.CreateEntityFromTemplate("menu/player_score_text");
            ANCHOR(texts[i].score)->parent = game->playerButtons[i];

            texts[i].bet = theEntityManager.CreateEntityFromTemplate("menu/player_bet_text");
            ANCHOR(texts[i].bet)->parent = game->playerButtons[i];

            texts[i].ready = theEntityManager.CreateEntityFromTemplate("menu/player_ready_text");
            ANCHOR(texts[i].ready)->parent = game->playerButtons[i];

            if (i % 2) {
                for (int j=0; j<3; j++) {
                    ANCHOR(texts[i].t[j])->anchor = -ANCHOR(texts[i].t[j])->anchor;
                    TEXT(texts[i].t[j])->positioning = 1 - TEXT(texts[i].t[j])->positioning;
                }
            }
        }
        playButton = theEntityManager.CreateEntityFromTemplate("menu/play_button");

        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_north"));
        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_south"));
        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_west"));
        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_east"));
    }

    void updateBet(int index) {
        char* tmp = (char*) alloca(40);
        sprintf(tmp, game->gameThreadContext->localizeAPI->text("bet").c_str(), 1 + game->playerActive[index]);
        TEXT(texts[index].bet)->text = tmp;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum from) override {
        {
            char* scoreText = (char*)alloca(50);
            for (int i=0; i<4; i++) {
                if (game->playerActive[i] >= 0) {
                    BUTTON(game->playerButtons[i])->enabled =
                        TEXT(texts[i].score)->show =
                        TEXT(texts[i].bet)->show =
                        RENDERING(game->playerButtons[i])->show = true;
                    updateBet(i);
                } else {
                    BUTTON(game->playerButtons[i])->enabled =
                        TEXT(texts[i].score)->show =
                        TEXT(texts[i].bet)->show =
                        RENDERING(game->playerButtons[i])->show = false;
                }

                sprintf(scoreText, game->gameThreadContext->localizeAPI->text("score").c_str(), game->score[i]);
                TEXT(texts[i].score)->text = scoreText;
            }
        }
        TEXT(playButton)->show = true;
        BUTTON(playButton)->enabled = true;
        RENDERING(playButton)->show = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(playButton)->clicked) {
            for (int i=0; i<4; i++) {
                RENDERING(game->playerButtons[i])->color.a = 0.2;
                TEXT(texts[i].bet)->show =
                    TEXT(texts[i].score)->show = false;
                ready[i] = (game->playerActive[i] < 0);
                if (ready[i]) {
                    BUTTON(game->playerButtons[i])->enabled = false;
                    TEXT(texts[i].ready)->show = false;
                } else {
                    BUTTON(game->playerButtons[i])->enabled = true;
                    TEXT(texts[i].ready)->show = true;
                }
            }

            TEXT(playButton)->show = false;
            RENDERING(playButton)->show = false;

            game->beesPopping(playButton);
            return Scene::InGame;
        }

        for (int i=0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                game->playerActive[i] = (game->playerActive[i] + 1) % 10;
                updateBet(i);
                game->beesPopping(game->playerButtons[i]);
            }
        }
        return Scene::GameStart;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        game->bees.clear();
        game->selected.clear();

        int beeCount = 50;
        game->parameters.get("Game", "bee_count", &beeCount);
        for (int i=0; i<beeCount; i++) {
            Entity bee = theEntityManager.CreateEntityFromTemplate("game/bee");
            RENDERING(bee)->show = true;
            AUTONOMOUS(bee)->walls = walls;
            game->bees.push_back(bee);
        }


        size_t count = 0;
        for (int i=0; i<4; i++) {
            if (game->playerActive[i] >= 0) {
                count += game->playerActive[i] + 1;
            }
        }

        int n[100];
        int index = 0;

        while (game->selected.size() < count) {
            Random::N_Ints(100, n, 0, game->bees.size() - 1);

            for (int i=0; i<100 && game->selected.size() < count; i++) {
                const auto bee = game->bees[n[i]];
                if (std::find(game->selected.begin(), game->selected.end(), bee) == game->selected.end()) {
                    game->selected.push_back(bee);
                }
            }
        }

        int idx = 0;
        for (int i=0; i<4; i++) {
            for (int j=0; j<game->playerActive[i] + 1; j++) {
                Entity h = theEntityManager.CreateEntityFromTemplate("game/bee_highlight");
                ANCHOR(h)->parent = game->selected[idx++];
                TRANSFORM(h)->size = TRANSFORM(ANCHOR(h)->parent)->size * 2.0f;
                RENDERING(h)->color = game->playerColors[i + 1];
                highlights.push_back(h);

                LOGI("Bee " << game->selected[idx - 1] << " assigned to player " << i);
            }
        }
    }

    bool updatePreExit(Scene::Enum, float) override {
        for (int i=0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                ready[i] = !ready[i];
                if (ready[i]) {
                    BUTTON(game->playerButtons[i])->enabled = false;
                    TEXT(texts[i].ready)->show = false;
                }
            }
        }

        for (int i=0; i<4; i++) {
            if (!ready[i])
                return false;
        }
        return true;
    }

    void onExit(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            RENDERING(game->playerButtons[i])->color.a = 1;
            BUTTON(game->playerButtons[i])->enabled =
                TEXT(texts[i].bet)->show =
                TEXT(texts[i].score)->show =
                TEXT(texts[i].ready)->show =
                RENDERING(game->playerButtons[i])->show = false;
        }
        TEXT(playButton)->show =
            BUTTON(playButton)->enabled =
            RENDERING(playButton)->show = false;

        for (auto b: game->bees) {
            PHYSICS(b)->mass = 1;
        }
        for (auto h: highlights) {
            AUTO_DESTROY(h)->type = AutoDestroyComponent::LIFETIME;
        }
        highlights.clear();
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameStartSceneHandler(PrototypeGame* game) {
        return new GameStartScene(game);
    }
}
