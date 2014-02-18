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

#include "systems/PlayerSystem.h"

#include "base/EntityManager.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "api/NetworkAPI.h"
#include "api/linux/NetworkAPILinuxImpl.h"

#include "PrototypeGame.h"

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
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onPreEnter(Scene::Enum) override {
        RENDERING(startBtn)->show = TEXT(startBtn)->show = TEXT(networkStatus)->show = true;

        net = static_cast<NetworkAPILinuxImpl*>(game->gameThreadContext->networkAPI);
        net->init();
        net->login(game->nickName, "127.0.0.1");

        for (int i=0; i<4; i++) {
            Entity p = theEntityManager.CreateEntityFromTemplate("menu/net_player");
            players.push_back(p);
            TRANSFORM(p)->position.x += TRANSFORM(p)->size.x * 1.1 * i;
        }
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        // update button
        const auto state = game->gameThreadContext->networkAPI->getStatus();
        switch (state) {
            case NetworkStatus::ConnectingToLobby:
                TEXT(networkStatus)->text = "ConnectingToLobby";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::ConnectedToLobby:
                TEXT(networkStatus)->text = "ConnectedToLobby";
                BUTTON(startBtn)->enabled = false;
                break;
            case NetworkStatus::ConnectionToLobbyFailed:
                TEXT(networkStatus)->text = "ConnectionToLobbyFailed";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::LoginInProgress:
                TEXT(networkStatus)->text = "LoginInProgress";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::Logged:
                TEXT(networkStatus)->text = "Logged";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::LoginFailed:
                TEXT(networkStatus)->text = "LoginFailed";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::CreatingRoom:
                TEXT(networkStatus)->text = "CreatingRoom";
                BUTTON(startBtn)->enabled = false;
                break;
            case NetworkStatus::InRoomAsMaster:
                TEXT(networkStatus)->text = "InRoomAsMaster";
                BUTTON(startBtn)->enabled = true;
                break;
            case NetworkStatus::JoiningRoom:
                TEXT(networkStatus)->text = "JoiningRoom";
                BUTTON(startBtn)->enabled = false;
                break;
            case NetworkStatus::ConnectedToServer:
                TEXT(networkStatus)->text = "ConnectedToServer";
                BUTTON(startBtn)->enabled = true;
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

        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        TEXT(startBtn)->text = "Loading...";
        BUTTON(startBtn)->enabled = false;            

        game->player = theEntityManager.CreateEntityFromTemplate("game/player");
        PLAYER(game->player)->name = game->nickName;
    }

    bool updatePreExit(Scene::Enum, float) override {
        if (net->getStatus() != NetworkStatus::ConnectedToServer) {
            // only proceed when all players exist
            return (thePlayerSystem.entityCount()
                >= net->getPlayersInRoom().size());
        } else {
            return true;
        }
    }

    void onExit(Scene::Enum) override {
        if (net->getStatus() != NetworkStatus::ConnectedToServer) {
            game->initGame();
        }
        RENDERING(startBtn)->show =
            TEXT(startBtn)->show =
            TEXT(networkStatus)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
