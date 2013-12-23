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

#include "systems/CameraSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"
#include "WeaponSystem.h"
#include "SoldierSystem.h"
#include "BulletSystem.h"
#include "MessageSystem.h"
#include "PlayerSystem.h"
#include "util/Random.h"

#define ZOOM 1

#include <ostream>
#include <fstream>
#if SAC_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "base/StateMachine.inl"
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
    sceneStateMachine.registerState(Scene::Active, Scene::CreateActiveSceneHandler(this), "Scene::Active");
    sceneStateMachine.registerState(Scene::Paused, Scene::CreatePausedSceneHandler(this), "Scene::Paused");
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

    WeaponSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(WeaponSystem::GetInstancePointer());
    SoldierSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(SoldierSystem::GetInstancePointer());
    BulletSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(BulletSystem::GetInstancePointer());
    MessageSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(MessageSystem::GetInstancePointer());
    PlayerSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(PlayerSystem::GetInstancePointer());

    Game::sacInit(windowW, windowH);

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

    theCollisionSystem.worldSize = glm::vec2(PlacementHelper::ScreenSize.x, PlacementHelper::ScreenSize.x);

    cameraMoveManager.init(camera, TRANSFORM(camera)->size, 5, 
        glm::vec4(glm::vec2(-PlacementHelper::ScreenSize.x * 0.5f), glm::vec2(PlacementHelper::ScreenSize.x * 0.5f)));
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

void PrototypeGame::initGame(int playerCount) {
    for (int i=0; i<30; i++) {
        theEntityManager.CreateEntityFromTemplate("block");
    }

    const std::string weapons[] = {"shotgun", "machinegun"};
    for (int i=0; i<playerCount; i++) {
        Color c = Color::random();
        for (int i=0; i<4; i++) {
            Entity p = theEntityManager.CreateEntityFromTemplate("p");
            RENDERING(p)->color = c;
            SOLDIER(p)->weapon = theEntityManager.CreateEntityFromTemplate(weapons[Random::Int(0, 1)]);
            players.push_back(p);
            TRANSFORM(p)->position.x += TRANSFORM(p)->size.x * 1.1 * i;
        }
    }
}
