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

struct VictoryScene : public StateHandler<Scene::Enum> {
    BzzzGame* game;

    Entity done[4];
    bool ready[4];

    VictoryScene(BzzzGame* game) : StateHandler<Scene::Enum>() {
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
    void onEnter(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            if (game->playerActive[i] >= 0) {
                BUTTON(game->playerButtons[i])->enabled =
                    RENDERING(game->playerButtons[i])->show = true;
                TEXT(done[i])->text = game->gameThreadContext->localizeAPI->text( (game->score[i] >= 10) ? "victory":"defeat");
                TEXT(done[i])->show = true;
                ready[i] = false;
            } else {
                BUTTON(game->playerButtons[i])->enabled =
                    RENDERING(game->playerButtons[i])->show =
                    TEXT(done[i])->show = false;
                ready[i] = true;
            }
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        for (int i = 0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                ready[i] = true;
                BUTTON(game->playerButtons[i])->enabled =
                    RENDERING(game->playerButtons[i])->show =
                    TEXT(done[i])->show = false;
            }
        }

        bool everyoneReady = true;
        for (int i=0; i<4 && everyoneReady; i++) {
            everyoneReady &= ready[i];
        }
        if (everyoneReady)
            return Scene::Menu;

        return Scene::Victory;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            TEXT(done[i])->show = false;
            BUTTON(game->playerButtons[i])->enabled =
                RENDERING(game->playerButtons[i])->show = false;
            RENDERING(game->playerButtons[i])->color.a = 1.0;
        }
    }

    void onExit(Scene::Enum) override {

    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateVictorySceneHandler(BzzzGame* game) {
        return new VictoryScene(game);
    }
}
