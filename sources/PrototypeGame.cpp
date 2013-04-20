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

#if SAC_INGAME_EDITORS
#include "util/PrototypeDebugConsole.h"
#endif

#define ZOOM 1


PrototypeGame::PrototypeGame() : Game() {
   state2manager.insert(std::make_pair(State::Logo, new LogoState(this)));
   state2manager.insert(std::make_pair(State::Menu, new MenuState(this)));
   state2manager.insert(std::make_pair(State::SocialCenter, new SocialCenterState(this)));
}

bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
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

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...")
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    gameThreadContext->storageAPI->init(gameThreadContext->assetAPI, "Prototype");
    ScoreStorageProxy ssp;
    gameThreadContext->storageAPI->createTable((IStorageProxy*)&ssp);


    theRenderingSystem.loadAtlas("font", true);
    // init font
    loadFont(renderThreadContext->assetAPI, "typo");
    std::list<std::string> files = gameThreadContext->assetAPI->listContent(".atlas");
    for(auto it=files.begin(); it!=files.end(); ++it)
        std::cout << *it << std::endl;
    LOGI("SAC engine initialisation done.")
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...")
    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->setup();
    }

    overrideNextState = State::Invalid;
    currentState = State::Menu;

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

#if SAC_INGAME_EDITORS
    PrototypeDebugConsole::init();
#endif

    quickInit();
    LOGI("PrototypeGame initialisation done.")
}

void PrototypeGame::quickInit() {
    state2manager[currentState]->willEnter(State::Invalid);
    state2manager[currentState]->enter(State::Invalid);
}

void PrototypeGame::changeState(State::Enum newState) {
    if (newState == currentState)
        return;

    state2manager[currentState]->willExit(newState);
    state2manager[currentState]->exit(newState);
    state2manager[newState]->willEnter(currentState);
    state2manager[newState]->enter(currentState);
    currentState = newState;
}

void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool) {

}

void PrototypeGame::tick(float dt) {
    if (currentState != State::Transition) {
        State::Enum newState = state2manager[currentState]->update(dt);

        if (newState != currentState) {
            state2manager[currentState]->willExit(newState);
            transitionManager.enter(state2manager[currentState], state2manager[newState]);
            currentState = State::Transition;
        }
    } else if (transitionManager.transitionFinished(&currentState)) {
        transitionManager.exit();
        state2manager[currentState]->enter(transitionManager.from->state);
    }

    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->backgroundUpdate(dt);
    }
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
