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

struct GameStartScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    Entity texts[4];
    Entity playButton;
    bool ready[4];
    std::vector<Entity> highlights;

    std::list<Entity> walls;

    GameStartScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        for (int i=0; i<4; i++) {
            texts[i] = theEntityManager.CreateEntityFromTemplate("menu/player_button_text");
            ANCHOR(texts[i])->parent = game->playerButtons[i];
            ANCHOR(texts[i])->position *= glm::normalize(-TRANSFORM(game->playerButtons[i])->position);
            if (ANCHOR(texts[i])->position.x > 0) {
                TEXT(texts[i])->positioning = TextComponent::RIGHT;
            } else {
                TEXT(texts[i])->positioning = TextComponent::LEFT;
            }
        }
        playButton = theEntityManager.CreateEntityFromTemplate("menu/play_button");

        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_north"));
        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_south"));
        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_west"));
        walls.push_back(theEntityManager.CreateEntityFromTemplate("game/wall_east"));
    }

    void updateButton(int index) {
        char* tmp = (char*) alloca(40);

        if (ANCHOR(texts[index])->position.x > 0) {
            sprintf(tmp, "%02d - %02d", game->score[index], 1 + game->playerActive[index]);
        } else {
            sprintf(tmp, "%02d - %02d", 1 + game->playerActive[index], game->score[index]);
        }
        TEXT(texts[index])->text = tmp;
    }

    void updateReady(int index) {
        if (ready[index]) {
            BUTTON(game->playerButtons[index])->enabled = false;
            TEXT(texts[index])->text = "";
        } else {
            TEXT(texts[index])->text = "tap";
        }
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            if (game->playerActive[i] >= 0) {
                BUTTON(game->playerButtons[i])->enabled =
                    TEXT(texts[i])->show =
                    RENDERING(game->playerButtons[i])->show = true;
                updateButton(i);
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
        for (int i=0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                game->playerActive[i] = (game->playerActive[i] + 1) % 10;
                updateButton(i);
            }
        }

        if (BUTTON(playButton)->clicked) {
            for (int i=0; i<4; i++) {
                ready[i] = (game->playerActive[i] < 0);
                updateReady(i);
            }
            TEXT(playButton)->text = "tap when ready";
            RENDERING(playButton)->show = false;
            return Scene::InGame;
        }

        return Scene::GameStart;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        game->bees.clear();
        game->selected.clear();

        for (int i=0; i<50; i++) {
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
        Random::N_Ints(100, n, 0, game->bees.size() - 1);
        while (game->selected.size() != count) {
            game->selected.push_back(game->bees[n[index++]]);
            std::unique(game->selected.begin(), game->selected.end());

            LOGF_IF(index == 100, "Bleuarg, not enough random values");
        }

        int idx = 0;
        for (int i=0; i<4; i++) {
            for (int j=0; j<game->playerActive[i] + 1; j++) {
                Entity h = theEntityManager.CreateEntityFromTemplate("game/bee_highlight");
                ANCHOR(h)->parent = game->selected[idx++];
                TRANSFORM(h)->size = TRANSFORM(ANCHOR(h)->parent)->size * 2.0f;
                RENDERING(h)->color = game->playerColors[i + 1];
                highlights.push_back(h);
            }
        }
    }

    bool updatePreExit(Scene::Enum, float) override {
        for (int i=0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                ready[i] = !ready[i];
                updateReady(i);
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
            BUTTON(game->playerButtons[i])->enabled =
                TEXT(texts[i])->show =
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
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameStartSceneHandler(PrototypeGame* game) {
        return new GameStartScene(game);
    }
}
