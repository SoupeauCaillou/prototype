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
#include <sstream>

#include <base/Log.h>
#include <base/TouchInputManager.h>
#include <base/MathUtil.h>
#include <base/EntityManager.h>
#include <base/TimeUtil.h>
#include <base/PlacementHelper.h>
#include "util/IntersectionUtil.h"

#include "api/NameInputAPI.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/TaskAISystem.h"
#include "systems/MusicSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/MorphingSystem.h"

#include "systems/FighterSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/EquipmentSystem.h"
#include "systems/SlotSystem.h"
#include "systems/PickableSystem.h"

#include "GameMaster.h"
#include <cmath>

PrototypeGame::PrototypeGame(AssetAPI* ast, NameInputAPI* inputUI, LocalizeAPI* lAPI, AdAPI* ad) : Game() {
	asset = ast;

   overrideNextState = State::Invalid;
   currentState = State::Logo;
   state2manager.insert(std::make_pair(State::Logo, new LogoStateManager(this)));
   state2manager.insert(std::make_pair(State::Menu, new MenuStateManager(this)));
   state2manager.insert(std::make_pair(State::Equipment, new EquipmentStateManager(this)));
   state2manager.insert(std::make_pair(State::AssignColor, new AssignColorStateManager(this)));

   FighterSystem::CreateInstance();
   PlayerSystem::CreateInstance();
   EquipmentSystem::CreateInstance();
   SlotSystem::CreateInstance();
   PickableSystem::CreateInstance();
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theRenderingSystem.loadAtlas("alphabet", true);
    theRenderingSystem.loadAtlas("fighter");
    theRenderingSystem.loadAtlas("equipment");
    theRenderingSystem.loadEffectFile("selected.fs");

    // init font
    loadFont(asset, "typo");
}

void PrototypeGame::init(const uint8_t* in, int size) {
    Color::nameColor(Color(172.0/255,39.0/255,39.0/255), "color_p1");
    Color::nameColor(Color(53.0/255,102.0/255,45.0/255), "color_p2");

    Color::nameColor(Color(232.0/255,176.0/255,14.0/255), "group_0");
    Color::nameColor(Color(25.0/255,43.0/255,154.0/255), "group_1");
    Color::nameColor(Color(154.0/255,89.0/255,25.0/255), "group_2");
    Color::nameColor(Color(118.0/255,191.0/255,198.0/255), "group_3");

    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->setup();
    }

    currentState = State::Menu;

    // ...
    stateChangeListeners.push_back(new GameMaster());

    quickInit();
}

void PrototypeGame::quickInit() {
    state2manager[currentState]->willEnter(State::Invalid);
    state2manager[currentState]->enter(State::Invalid);

    for(unsigned i=0; i<stateChangeListeners.size(); i++) {
        stateChangeListeners[i]->stateChanged(State::Invalid, currentState);
    }
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

void PrototypeGame::togglePause(bool activate) {

}

void PrototypeGame::tick(float dt) {
    State::Enum oldState = currentState;
    if (overrideNextState != State::Invalid) {
        changeState(overrideNextState);
        overrideNextState = State::Invalid;
    }

    if (State::Transition != currentState) {
        State::Enum newState = state2manager[currentState]->update(dt);

        if (newState != currentState) {
            state2manager[currentState]->willExit(newState);
            transitionManager.enter(state2manager[currentState], state2manager[newState]);
            currentState = State::Transition;
        }
    } else if (transitionManager.transitionFinished(&currentState)) {
        transitionManager.exit();
        state2manager[currentState]->enter(transitionManager.from->state);
        // notify state change
        for(unsigned i=0; i<stateChangeListeners.size(); i++) {
            stateChangeListeners[i]->stateChanged(transitionManager.from->state, currentState);
        }
    }

    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->backgroundUpdate(dt);
    }
    if (oldState != currentState) {
        // notify state change
        for(unsigned i=0; i<stateChangeListeners.size(); i++) {
            stateChangeListeners[i]->stateChanged(oldState, currentState);
        }
    }
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
