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
#include "TeamSystem.h"
#include "util/Random.h"

#include "MessageSystem.h"

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

        net = static_cast<NetworkAPILinuxImpl*>(game->gameThreadContext->networkAPI);
        net->init();
        net->login(game->nickName, game->serverIp);

        for (int i=0; i<4; i++) {
            Entity p = theEntityManager.CreateEntityFromTemplate("menu/net_player");
            if (i > 0)
                TRANSFORM(p)->position = TRANSFORM(players[i-1])->position + TRANSFORM(players[i-1])->size * glm::vec2(1.1f, .0f);
            players.push_back(p);
        }
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {

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

        for (auto p: theMessageSystem.getAllComponents()) {
            switch (p.second->type) {
                case Message::ChangeState: {
                    if (p.second->newState == Scene::GameStart)
                        return Scene::GameStart;
                }
            }
        }

        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        game->isGameHost = (net->getStatus() == NetworkStatus::Logged || net->getStatus() == NetworkStatus::ConnectingToLobby || net->getStatus() == NetworkStatus::InRoomAsMaster);
        if (game->isGameHost)
            game->initGame(net->getPlayersInRoom(), game->isGameHost);
    }

    bool updatePreExit(Scene::Enum, float) override {
        if (game->isGameHost) {
            return true;
        }
        else {
            if (theTeamSystem.entityCount() > 0) {
                game->initGame(net->getPlayersInRoom(), game->isGameHost);
                return true;
            }
        }

        return false;
    }

    void onExit(Scene::Enum) override {            
        // notify everyone
        Entity msg = theEntityManager.CreateEntityFromTemplate("message");
        MESSAGE(msg)->type = Message::ChangeState;
        MESSAGE(msg)->newState = Scene::GameStart;

        RENDERING(startBtn)->show =
            TEXT(startBtn)->show =
            TEXT(networkStatus)->show =
            BUTTON(startBtn)->enabled = false;
        for (auto p: players) {
            TEXT(p)->show = RENDERING(p)->show = false;
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
