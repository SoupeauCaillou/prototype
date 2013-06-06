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
#include "base/PlacementHelper.h"

#include "util/IntersectionUtil.h"
#include "util/ScoreStorageProxy.h"

#include "api/StorageAPI.h"

#include "systems/TransformationSystem.h"
#include "systems/SoldierSystem.h"
#include "systems/ActionSystem.h"

#if SAC_INGAME_EDITORS
#include "util/PrototypeDebugConsole.h"
#endif

#define ZOOM 1


PrototypeGame::PrototypeGame(int, char**) : Game(), grid(39, 27, 1.1) {
    nickname = "anonymous";
    serverIp = "127.0.0.1";

    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::Connecting, Scene::CreateConnectingSceneHandler(this), "Scene::Connecting");
    sceneStateMachine.registerState(Scene::SocialCenter, Scene::CreateSocialCenterSceneHandler(this), "Scene::SocialCenter");
    sceneStateMachine.registerState(Scene::SelectCharacter, Scene::CreateSelectCharacterSceneHandler(this), "Scene::SelectCharacter");
    sceneStateMachine.registerState(Scene::SelectAction, Scene::CreateSelectActionSceneHandler(this), "Scene::SelectAction");
    sceneStateMachine.registerState(Scene::ExecuteAction, Scene::CreateExecuteActionSceneHandler(this), "Scene::ExecuteAction");
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

    SoldierSystem::CreateInstance();
    ActionSystem::CreateInstance();
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

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

    grid.doForEachCell([this] (const GridPos& p) -> void {
        Entity e = theEntityManager.CreateEntity("gridcell",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
        TRANSFORM(e)->position = this->grid.gridPosToPosition(p);
        grid.addEntityAt(e, p);
    });

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
