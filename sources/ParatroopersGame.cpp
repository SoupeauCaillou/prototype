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
#include "ParatroopersGame.h"

#include "systems/PlaneSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/ParachuteSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/DCASystem.h"

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

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/TaskAISystem.h"
#include "systems/MusicSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/CameraSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/GraphSystem.h"

#include "systems/BulletSystem.h"

#if SAC_INGAME_EDITORS
#include "util/ParatroopersDebugConsole.h"
#endif

#define ZOOM 1


ParatroopersGame::ParatroopersGame() : Game() {
    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::SocialCenter, Scene::CreateSocialCenterSceneHandler(this), "Scene::SocialCenter");
    sceneStateMachine.registerState(Scene::Test, Scene::CreateTestSceneHandler(this), "Scene::Test");
}

bool ParatroopersGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        case ContextAPI::Localize:
        case ContextAPI::Communication:
        case ContextAPI::Storage:
            return true;
        default:
            return false;
    }
}

void ParatroopersGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...")
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    gameThreadContext->storageAPI->init(gameThreadContext->assetAPI, "Paratroopers");
    ScoreStorageProxy ssp;
    gameThreadContext->storageAPI->createTable((IStorageProxy*)&ssp);


    theRenderingSystem.loadAtlas("logo", true);
    theRenderingSystem.loadAtlas("font", true);
    // init font
    loadFont(renderThreadContext->assetAPI, "typo");
    std::list<std::string> files = gameThreadContext->assetAPI->listContent(".atlas");
    for(auto it=files.begin(); it!=files.end(); ++it)
        LOGI("atlas file: " << *it);

    LOGI("SAC engine initialisation done.")
}

void ParatroopersGame::init(const uint8_t*, int) {
    LOGI("ParatroopersGame initialisation begins...")
    // sceneStateMachine.setup(Scene::Menu);
    // sceneStateMachine.reEnterCurrentState();
    PlaneSystem::CreateInstance();
    PlayerSystem::CreateInstance();
    BulletSystem::CreateInstance();
    ParatrooperSystem::CreateInstance();
    ParachuteSystem::CreateInstance();
    DCASystem::CreateInstance();
    // default camera
    camera = theEntityManager.CreateEntity("camera1");
    ADD_COMPONENT(camera, Transformation);
    TRANSFORM(camera)->size = glm::vec2(theRenderingSystem.screenW * ZOOM, theRenderingSystem.screenH * ZOOM);
    TRANSFORM(camera)->position = glm::vec2(0, 0);
    TRANSFORM(camera)->z = 1;
    ADD_COMPONENT(camera, Camera);
    CAMERA(camera)->enable = true;
    CAMERA(camera)->order = 2;
    CAMERA(camera)->id = 0;
    CAMERA(camera)->clearColor = Color(125.0/255, 150./255.0, 0.);

    sceneStateMachine.setup(Scene::Menu);
    sceneStateMachine.reEnterCurrentState();

#if SAC_INGAME_EDITORS
    ParatroopersDebugConsole::init(this);
#endif

    quickInit();
    LOGI("ParatroopersGame initialisation done.")
}

void ParatroopersGame::quickInit() {
    sceneStateMachine.reEnterCurrentState();
}

void ParatroopersGame::backPressed() {
}

void ParatroopersGame::togglePause(bool) {

}

void ParatroopersGame::tick(float dt) {
    if (dt > 0) {
        sceneStateMachine.update(dt);
        thePlaneSystem.Update(dt);
        theDCASystem.Update(dt);
        theBulletSystem.Update(dt);
        theParatrooperSystem.Update(dt);
        theParachuteSystem.Update(dt);
    }
}

bool ParatroopersGame::willConsumeBackEvent() {
    return false;
}