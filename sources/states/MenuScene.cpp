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
#include "base/TouchInputManager.h"
#include "systems/ButtonSystem.h"
#include "systems/SpotSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "api/NetworkAPI.h"
#include "api/linux/NetworkAPILinuxImpl.h"

#include "PrototypeGame.h"
#include "SoldierSystem.h"
#include "util/Random.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity startBtn, networkStatus, createRoom, acceptInvite;
    std::vector<Entity> players;
    NetworkAPILinuxImpl* net;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        startBtn = theEntityManager.CreateEntityFromTemplate("menu/startbtn");
        networkStatus = theEntityManager.CreateEntityFromTemplate("menu/network_status");
        createRoom = theEntityManager.CreateEntityFromTemplate("menu/create_room");
        acceptInvite = theEntityManager.CreateEntityFromTemplate("menu/accept_invite");

        theRenderingSystem.shapes.push_back(Polygon());
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onPreEnter(Scene::Enum) override {
        RENDERING(startBtn)->show = TEXT(startBtn)->show = TEXT(networkStatus)->show = true;
        BUTTON(startBtn)->enabled = true;

#if 1
        net = static_cast<NetworkAPILinuxImpl*>(game->gameThreadContext->networkAPI);
        net->init();
        net->login(game->nickName);
#endif

        for (int i=0; i<4; i++) {
            players.push_back(theEntityManager.CreateEntityFromTemplate("menu/net_player"));
        }
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
#if 1
        // update button
        const auto state = game->gameThreadContext->networkAPI->getStatus();
        switch (state) {
            case NetworkStatus::ConnectingToLobby:
                TEXT(networkStatus)->text = "ConnectingToLobby";
                break;
            case NetworkStatus::ConnectedToLobby:
                TEXT(networkStatus)->text = "ConnectedToLobby";
                break;
            case NetworkStatus::ConnectionToLobbyFailed:
                TEXT(networkStatus)->text = "ConnectionToLobbyFailed";
                break;
            case NetworkStatus::LoginInProgress:
                TEXT(networkStatus)->text = "LoginInProgress";
                break;
            case NetworkStatus::Logged:
                TEXT(networkStatus)->text = "Logged";
                break;
            case NetworkStatus::LoginFailed:
                TEXT(networkStatus)->text = "LoginFailed";
                break;
            case NetworkStatus::CreatingRoom:
                TEXT(networkStatus)->text = "CreatingRoom";
                break;
            case NetworkStatus::InRoomAsMaster:
                TEXT(networkStatus)->text = "InRoomAsMaster";
                break;
            case NetworkStatus::JoiningRoom:
                TEXT(networkStatus)->text = "JoiningRoom";
                break;
            case NetworkStatus::ConnectedToServer:
                TEXT(networkStatus)->text = "ConnectedToServer";
                break;
        }

        TEXT(createRoom)->show =
            RENDERING(createRoom)->show =
            BUTTON(createRoom)->enabled = (state == NetworkStatus::Logged);

        TEXT(acceptInvite)->show =
            RENDERING(acceptInvite)->show =
            BUTTON(acceptInvite)->enabled = (state == NetworkStatus::Logged && net->getPendingInvitationCount() > 0);

        {
            auto playersInRoom = net->getPlayersInRoom();
            int i=0;
            for (auto it=playersInRoom.begin(); it != playersInRoom.end(); ++it, i++) {
                TEXT(players[i])->show = RENDERING(players[i])->show = true;
                std::stringstream s;
                s << it->first << ':';
                switch (it->second) {
                    case NetworkStatus::InRoomAsMaster:
                        s << " master";
                        break;
                    case NetworkStatus::JoiningRoom:
                        s << " joining";
                        break;
                    case NetworkStatus::ConnectedToServer:
                        s << " connected";
                        break;
                    default:
                        s << " ERROR";
                }
                TEXT(players[i])->text = s.str();
            }
        }

        if (BUTTON(startBtn)->clicked) {
            return Scene::GameStart;
        }

        if (BUTTON(createRoom)->clicked) {
            net->createRoom();
        }

        if (BUTTON(acceptInvite)->clicked) {
            net->acceptInvitation();
        }
#endif
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
        }

        RENDERING(startBtn)->show =
            TEXT(startBtn)->show =
            TEXT(networkStatus)->show =
            BUTTON(startBtn)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
