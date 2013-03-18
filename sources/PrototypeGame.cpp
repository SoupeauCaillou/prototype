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

#include <cmath>

PrototypeGame::PrototypeGame() : Game() {
   state2manager.insert(std::make_pair(State::Logo, new LogoStateManager(this)));
   state2manager.insert(std::make_pair(State::Menu, new MenuStateManager(this)));
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theRenderingSystem.loadAtlas("alphabet", true);
    //-theRenderingSystem.loadAtlas("logo", false);

    // init font
    loadFont(gameThreadContext->assetAPI, "typo");
}

void PrototypeGame::init(const uint8_t*, int) {
    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->setup();
    }

    overrideNextState = State::Invalid;
    currentState = State::Menu;

    quickInit();
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
    if (overrideNextState != State::Invalid) {
        changeState(overrideNextState);
        overrideNextState = State::Invalid;
    }

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
    { static int i=0;
        //std::cout << "Nombre d'entité = " << ++i << std::endl;

        Entity eq = theEntityManager.CreateEntity();
        ADD_COMPONENT(eq, Transformation);
        TRANSFORM(eq)->z = 0.5;
        TRANSFORM(eq)->size = Vector2(0.5,0.5);
        TRANSFORM(eq)->position = Vector2(MathUtil::RandomFloatInRange(-10, 10), MathUtil::RandomFloatInRange(-10, 10));
      ADD_COMPONENT(eq, Rendering);
        RENDERING(eq)->color = Color::random();
        RENDERING(eq)->hide = false;
        RENDERING(eq)->cameraBitMask = 0xffff;
        ADD_COMPONENT(eq, Physics);
        PHYSICS(eq)->mass = MathUtil::RandomFloat();
        PHYSICS(eq)->gravity = Vector2(0, -1);
    }
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
