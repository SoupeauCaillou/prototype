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

#include <cmath>

PrototypeGame::PrototypeGame(AssetAPI* ast, NameInputAPI* inputUI, LocalizeAPI* lAPI, AdAPI* ad) : Game() {
	asset = ast;

   overrideNextState = State::Invalid;
   currentState = State::Logo;
   state2manager.insert(std::make_pair(State::Logo, new LogoStateManager(this)));
   state2manager.insert(std::make_pair(State::Menu, new MenuStateManager(this)));
   state2manager.insert(std::make_pair(State::Equipment, new EquipmentStateManager(this)));

   FighterSystem::CreateInstance();
   PlayerSystem::CreateInstance();
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theRenderingSystem.loadAtlas("alphabet", true);
    theRenderingSystem.loadAtlas("fighter", true);

    // init font
    loadFont(asset, "typo");
}

static Entity createFighter() {
    Vector2 ref(363, 393);
    float scale = 1 / 200.0;
    Entity e = theEntityManager.CreateEntity();
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->size = ref * scale;
    TRANSFORM(e)->z = 0.5;
    ADD_COMPONENT(e, Fighter);

    std::string textures[] = {"head", "torso", "left_arm", "right_arm", "left_leg", "right_leg"};
    Vector2 positions[] = {
        Vector2(196, 65), Vector2(193, 181), Vector2(84, 168), Vector2(294, 162), Vector2(142, 315), Vector2(235, 316)
    };
    #define P(v) ((Vector2(-0.5, 0.5) * ref + Vector2(v.X, -v.Y)) * scale)
    for (int i=0; i<6; i++) {
        Entity member = FIGHTER(e)->members[i] = theEntityManager.CreateEntity();
        ADD_COMPONENT(member, Transformation);
        TRANSFORM(member)->parent = e;
        TRANSFORM(member)->size = theRenderingSystem.getTextureSize(textures[i]) * scale;
        TRANSFORM(member)->position = P(positions[i]);
        ADD_COMPONENT(member, Rendering);
        RENDERING(member)->texture = theRenderingSystem.loadTextureFile(textures[i]);
        RENDERING(member)->hide = false;
    }
    return e;
}

void PrototypeGame::init(const uint8_t* in, int size) {
    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->setup();
    }

    currentState = State::Menu;

    // Create 2 players entity
    Entity p1 = theEntityManager.CreateEntity();
    ADD_COMPONENT(p1, Player);
    Entity p2 = theEntityManager.CreateEntity();
    ADD_COMPONENT(p2, Player);

    for (int i=0; i<12; i++) {
        Entity f1 = createFighter();
        FIGHTER(f1)->player = p1;
        Entity f2 = createFighter();
        FIGHTER(f2)->player = p2;
    }

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
    }

    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->backgroundUpdate(dt);
    }
    if (currentState != oldState) {
        for(unsigned i=0; i<stateChangeListeners.size(); i++) {
            stateChangeListeners[i]->stateChanged(oldState, currentState);
        }
    }
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
