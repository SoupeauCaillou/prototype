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
#include "api/NetworkAPI.h"

#include "PrototypeGame.h"
#include "systems/ActionSystem.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity startBtn;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        startBtn = theEntityManager.CreateEntityFromTemplate("menu/startbtn");
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        RENDERING(startBtn)->show = TEXT(startBtn)->show = true;
        BUTTON(startBtn)->enabled = false;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        // update button
        switch (game->gameThreadContext->networkAPI->getStatus()) {
            case NetworkStatus::ConnectingToLobby:
                TEXT(startBtn)->text = "Connecting";
                break;
            case NetworkStatus::ConnectedToLobby:
                TEXT(startBtn)->text = "In lobby";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::None:
            case NetworkStatus::ConnectionToLobbyFailed:
                TEXT(startBtn)->text = "Single pl";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::ConnectingToServer:
                TEXT(startBtn)->text = "Connecting";
                break;
            case NetworkStatus::ConnectedToServer:
                TEXT(startBtn)->text = "Connected";
                BUTTON(startBtn)->enabled = true;
                RENDERING(startBtn)->color = Color(0, 1, 0);
                break;
            case NetworkStatus::ConnectionToServerFailed:
                LOGF("Failed to connect to server");
                break;
        }

        if (BUTTON(startBtn)->clicked)
            return Scene::GameStart;
        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
        // create game entities, if game master
        if (!game->gameThreadContext->networkAPI->isConnectedToAnotherPlayer() ||
            game->gameThreadContext->networkAPI->amIGameMaster()) {
            Entity orc = theEntityManager.CreateEntityFromTemplate("ingame/soldier");
            Entity action = theEntityManager.CreateEntityFromTemplate("ingame/action");
            ACTION(action)->orc = orc;

            // create 10 blocks
            for (int i=0; i<10; i++) {
                theEntityManager.CreateEntityFromTemplate("ingame/block");
            }
        }

        RENDERING(startBtn)->show =
            TEXT(startBtn)->show = 
            BUTTON(startBtn)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
