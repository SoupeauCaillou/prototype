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

#include "PrototypeGame.h"

#include "base/PlacementHelper.h"
#include "base/StateMachine.inl"

#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"
#include "util/DataFileParser.h"

#include "systems/PlayerSystem.h"
#include "systems/SoldierSystem.h"
#include "systems/FlickSystem.h"
#include "systems/KnightSystem.h"
#include "systems/ArcherSystem.h"
#include "systems/ProjectileSystem.h"
#include "systems/MessageSystem.h"

#include <ostream>
#include <fstream>
#if SAC_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

PrototypeGame::PrototypeGame(int argc, char** argv) : Game(), serverIp(""), nickName("johndoe"){
#if SAC_LINUX && SAC_DESKTOP
    char* nick = getlogin();
    if (nick)
        nickName = nick;
#endif

    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-server") == 0) {
            LOGF_IF(i + 1 >= argc, "Incorrect #args");
            serverIp = argv[++i];
        }
        else if (strcmp(argv[i], "-nick") == 0) {
            LOGF_IF(i + 1 >= argc, "Incorrect #args");
            nickName = argv[++i];
        }
    }
    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::GameStart, Scene::CreateGameStartSceneHandler(this), "Scene::GameStart");
    sceneStateMachine.registerState(Scene::InGame, Scene::CreateInGameSceneHandler(this), "Scene::InGame");
    sceneStateMachine.registerState(Scene::GameEnd, Scene::CreateGameEndSceneHandler(this), "Scene::GameEnd");
    LOGF_IF(sceneStateMachine.getStateCount() != (int)Scene::Count,
        "Missing " << (int)Scene::Count - sceneStateMachine.getStateCount() << " state handler(s)");
}

bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        case ContextAPI::Localize:
        case ContextAPI::Communication:
        case ContextAPI::Sound:
#if SAC_NETWORK
        case ContextAPI::Network:
#endif
        case ContextAPI::KeyboardInputHandler:
        case ContextAPI::WWW:
            return true;
        default:
            return false;
    }
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");

    PlayerSystem::CreateInstance();
    SoldierSystem::CreateInstance();
    FlickSystem::CreateInstance();
    KnightSystem::CreateInstance();
    ArcherSystem::CreateInstance();
    ProjectileSystem::CreateInstance();

    MessageSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(MessageSystem::GetInstancePointer());

    Game::sacInit(windowW, windowH);

    theCollisionSystem.worldSize = PlacementHelper::ScreenSize;
    PlacementHelper::GimpSize = glm::vec2(1280, 800);

    LOGI("SAC engine initialisation done.");
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    // default camera
    camera = theEntityManager.CreateEntityFromTemplate("camera");
    faderHelper.init(camera);

    sceneStateMachine.setup();
#if SAC_DEBUG
    sceneStateMachine.start(Scene::Menu);
#else
    sceneStateMachine.start(Scene::Logo);
#endif

    LOGI("PrototypeGame initialisation done.");

    if (!serverIp.empty()) {
        // init network connection
        auto* api = gameThreadContext->networkAPI;
        // connect to lobby
        api->connectToLobby(nickName, serverIp.c_str());
    } else {
        LOGI("Solo game");
    }
}

void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool) {

}

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}

void PrototypeGame::initGame() {
    for (int i=0; i<15; i++) {
        theEntityManager.CreateEntityFromTemplate("game/block");
    }

    if (thePlayerSystem.entityCount() == 1) {
        Entity ai = theEntityManager.CreateEntityFromTemplate("game/player");
        PLAYER(ai)->name = "ai";
    }

    LOGF_IF(thePlayerSystem.entityCount() != 2, "Only 2 player map");
    FileBuffer fb = gameThreadContext->assetAPI->loadAsset("placement.ini");
    DataFileParser dfp;
    dfp.load(fb, "placement.ini");

    const auto players = thePlayerSystem.RetrieveAllEntityWithComponent();
    for (int i=0; i<2; i++) {
        std::stringstream section;
        section << "team" << (i+1);

        int archers, knights;
        dfp.get(section.str(), "archer_count", &archers);
        dfp.get(section.str(), "knight_count", &knights);

        Color c;
        dfp.get(section.str(), "color", &c.r, 4);
        for (int j=0; j<archers; j++) {
            Entity a = theEntityManager.CreateEntityFromTemplate("game/archer");
            SOLDIER(a)->player = players[i];

            std::stringstream pos;
            pos << "pos_a" << (j+1);
            dfp.get(section.str(), pos.str(), &TRANSFORM(a)->position.x, 2);
            TRANSFORM(a)->position = PlacementHelper::GimpPositionToScreen(TRANSFORM(a)->position);
            RENDERING(a)->color = c;
        }

        for (int j=0; j<knights; j++) {
            Entity a = theEntityManager.CreateEntityFromTemplate("game/knight");
            SOLDIER(a)->player = players[i];

            std::stringstream pos;
            pos << "pos_k" << (j+1);
            dfp.get(section.str(), pos.str(), &TRANSFORM(a)->position.x, 2);
            TRANSFORM(a)->position = PlacementHelper::GimpPositionToScreen(TRANSFORM(a)->position);
            RENDERING(a)->color = c;
        }
    }

    theSoldierSystem.forEachEntityDo([] (Entity e) -> void {
        glm::vec2 d(TRANSFORM(e)->position);
        TRANSFORM(e)->rotation = glm::atan(d.y, d.x) + glm::pi<float>() * 0.5f;
    });
}