/*
	This file is part of RecursiveRunner.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	RecursiveRunner is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	RecursiveRunner is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "base/StateMachine.h"

#include "Scenes.h"

#include <systems/TransformationSystem.h>
#include <systems/ButtonSystem.h>
#include <systems/RenderingSystem.h>
#include <systems/TextRenderingSystem.h>

#include "api/NetworkAPI.h"

#include "PrototypeGame.h"

struct ConnectingScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity waitIcon, infoText, cancelBtn, startBtn;

    ConnectingScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        waitIcon = theEntityManager.CreateEntity("waitIcon",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("connecting/waitIcon"));
        infoText = theEntityManager.CreateEntity("infoText",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("connecting/infoText"));
        cancelBtn = theEntityManager.CreateEntity("cancelBtn",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("connecting/cancelBtn"));
        startBtn = theEntityManager.CreateEntity("startBtn",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("connecting/startBtn"));
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        TEXT_RENDERING(infoText)->show =
        TEXT_RENDERING(cancelBtn)->show =
        BUTTON(cancelBtn)->enabled =
        RENDERING(waitIcon)->show =
        RENDERING(cancelBtn)->show = true;

        BUTTON(startBtn)->enabled =
        RENDERING(startBtn)->show = false;

        // Start connection to lobby
        game->gameThreadContext->networkAPI->connectToLobby(game->nickname.c_str(), game->serverIp.c_str());
        TEXT_RENDERING(infoText)->text = "Connecting to lobby @" + game->serverIp;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        bool spin = true;

        if (BUTTON(cancelBtn)->clicked)
            return Scene::Menu;
        if (BUTTON(startBtn)->clicked)
            return Scene::Ingame;

        NetworkStatus::Enum status = game->gameThreadContext->networkAPI->getStatus();
        switch (status) {
            case NetworkStatus::None:
                TEXT_RENDERING(infoText)->text = "...";
                break;
            case NetworkStatus::ConnectingToLobby:
                TEXT_RENDERING(infoText)->text = "Connecting to lobby @" + game->serverIp;
                break;
            case NetworkStatus::ConnectionToLobbyFailed:
                spin = false;
                TEXT_RENDERING(infoText)->text = "Failed to connect to lobby @" + game->serverIp;
                break;
            case NetworkStatus::ConnectedToLobby:
                TEXT_RENDERING(infoText)->text = "Connected to lobby @" + game->serverIp;
                spin = false;
                break;
            case NetworkStatus::ConnectingToServer:
                TEXT_RENDERING(infoText)->text = "Connecting to game server...";
                break;
            case NetworkStatus::ConnectedToServer:
                TEXT_RENDERING(infoText)->text = "Connected to game server !";
                BUTTON(startBtn)->enabled = RENDERING(startBtn)->show = true;
                break;
            case NetworkStatus::ConnectionToServerFailed:
                spin = false;
                TEXT_RENDERING(infoText)->text = "Connected to game server !";
                break;
        }

        TRANSFORM(waitIcon)->rotation += spin * 5 * dt;

        return Scene::Connecting;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        TEXT_RENDERING(infoText)->show =
        TEXT_RENDERING(cancelBtn)->show =
        BUTTON(cancelBtn)->enabled =
        RENDERING(waitIcon)->show =
        BUTTON(startBtn)->enabled =
        RENDERING(startBtn)->show =
        RENDERING(cancelBtn)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateConnectingSceneHandler(PrototypeGame* game) {
        return new ConnectingScene(game);
    }
}
