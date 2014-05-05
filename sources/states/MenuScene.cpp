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
#include "systems/ButtonSystem.h"
#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "api/NetworkAPI.h"
#include "api/linux/NetworkAPILinuxImpl.h"

#include "PrototypeGame.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    Entity playButton;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
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
        }
        TEXT(playButton)->show = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        for (int i=0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                game->playerActive[i] = -1 - game->playerActive[i];

                RENDERING(game->playerButtons[i])->color =
                    (game->playerActive[i] >= 0) ?
                    game->playerColors[i+1] :
                    game->playerColors[0];
            }
        }

        bool atLeastOnePlayerActive = false;
        for (int i=0; i<4 & !atLeastOnePlayerActive; i++) {
            atLeastOnePlayerActive |= (game->playerActive[i] >= 0);
        }
        RENDERING(playButton)->show =
            TEXT(playButton)->show =
            BUTTON(playButton)->enabled = atLeastOnePlayerActive;

        if (BUTTON(playButton)->clicked) {
            return Scene::GameStart;
        }

        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            BUTTON(game->playerButtons[i])->enabled =
                RENDERING(game->playerButtons[i])->show = false;
        }
    }

    void onExit(Scene::Enum) override {
        BUTTON(playButton)->enabled =
            RENDERING(playButton)->show =
            TEXT(playButton)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
