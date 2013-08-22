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

#define ZOOM 1

#include <ostream>
#include <fstream>
#if SAC_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "systems/ActionSystem.h"
#include "systems/OrcSystem.h"
#include "systems/CollisionSystem.h"

#include "api/NetworkAPI.h"

PrototypeGame::PrototypeGame(int argc, char** argv) : Game(), serverIp(""), nickName("johndoe") {
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

    ActionSystem::CreateInstance();
    OrcSystem::CreateInstance();
    Game::sacInit(windowW, windowH);

    PlacementHelper::GimpSize = glm::vec2(1280, 800);

    theRenderingSystem.createFramebuffer("ui_fb", windowW, windowH);
    LOGI("SAC engine initialisation done.");
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    theCollisionSystem.worldSize = PlacementHelper::ScreenSize * 2.0f;
#if SAC_DEBUG
    sceneStateMachine.setup(Scene::Menu);
#else
    sceneStateMachine.setup(Scene::Logo);
#endif

    // default camera
    camera = theEntityManager.CreateEntity("camera",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("camera"));

    quickInit();
    LOGI("PrototypeGame initialisation done.");
}

void PrototypeGame::quickInit() {
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
