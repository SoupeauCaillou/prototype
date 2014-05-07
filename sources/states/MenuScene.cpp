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
#include "systems/ButtonSystem.h"
#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"

#include "BzzzGame.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    BzzzGame* game;

    Entity playButton;

    MenuScene(BzzzGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        playButton = theEntityManager.CreateEntityFromTemplate("menu/play_button");
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onPreEnter(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            BUTTON(game->playerButtons[i])->enabled =
                RENDERING(game->playerButtons[i])->show = true;
            game->score[i] = 0;
        }
        TEXT(playButton)->show = true;
        RENDERING(playButton)->show = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//

    Scene::Enum update(float) override {
        if (BUTTON(playButton)->clicked) {
            game->beesPopping(playButton);
            return Scene::GameStart;
        }
        for (int i=0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                game->playerActive[i] = -1 - game->playerActive[i];

                RENDERING(game->playerButtons[i])->color =
                    (game->playerActive[i] >= 0) ?
                    game->playerColors[i+1] :
                    game->playerColors[0];

                game->beesPopping(game->playerButtons[i]);
            }
        }

        bool atLeastOnePlayerActive = false;
        for (int i=0; i<4 && !atLeastOnePlayerActive; i++) {
            atLeastOnePlayerActive |= (game->playerActive[i] >= 0);
        }
        RENDERING(playButton)->show =
            TEXT(playButton)->show =
            BUTTON(playButton)->enabled = atLeastOnePlayerActive;

        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum to) override {
        BUTTON(playButton)->enabled =
            RENDERING(playButton)->show =
            TEXT(playButton)->show = false;
    }

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(BzzzGame* game) {
        return new MenuScene(game);
    }
}
