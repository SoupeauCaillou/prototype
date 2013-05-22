/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "PrototypeGame.h"
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "base/TouchInputManager.h"
#include "base/EntityManager.h"
#include "base/TimeUtil.h"
#include "base/PlacementHelper.h"

#include "util/IntersectionUtil.h"
#include "util/ScoreStorageProxy.h"

#include "api/StorageAPI.h"
#include "api/NetworkAPI.h"

#include "systems/InputSystem.h"

#if SAC_INGAME_EDITORS
#include "util/PrototypeDebugConsole.h"
#endif

#define ZOOM 1


PrototypeGame::PrototypeGame(int argc, char** argv) : Game() {
    nickname = "anonymous";
    serverIp = "127.0.0.1";

    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::Connecting, Scene::CreateConnectingSceneHandler(this), "Scene::Connecting");
    sceneStateMachine.registerState(Scene::Ingame, Scene::CreateIngameSceneHandler(this), "Scene::Ingame");
    sceneStateMachine.registerState(Scene::SocialCenter, Scene::CreateSocialCenterSceneHandler(this), "Scene::SocialCenter");
}

bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        case ContextAPI::Localize:
        case ContextAPI::Communication:
        case ContextAPI::Storage:
        case ContextAPI::Sound:
        case ContextAPI::Network:
        case ContextAPI::KeyboardInputHandler:
            return true;
        default:
            return false;
    }
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    InputSystem::CreateInstance();

    gameThreadContext->storageAPI->init(gameThreadContext->assetAPI, "Prototype");
    ScoreStorageProxy ssp;
    gameThreadContext->storageAPI->createTable((IStorageProxy*)&ssp);

    LOGI("SAC engine initialisation done.");
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");
    sceneStateMachine.setup(Scene::Logo);
    sceneStateMachine.reEnterCurrentState();

    // default camera
    camera = theEntityManager.CreateEntity("camera",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("camera"));

#if SAC_INGAME_EDITORS
    PrototypeDebugConsole::init(this);
#endif

    quickInit();
    LOGI("PrototypeGame initialisation done.");
}

void PrototypeGame::quickInit() {
    sceneStateMachine.reEnterCurrentState();
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
