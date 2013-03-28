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
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <base/TouchInputManager.h>
#include <base/EntityManager.h>
#include <base/TimeUtil.h>
#include <base/PlacementHelper.h>
#include "util/IntersectionUtil.h"

#include "api/NameInputAPI.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/AutoDestroySystem.h"
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
#include "systems/CameraSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/GraphSystem.h"
#include "api/NetworkAPI.h"

#include <GL/glfw.h>

#define ZOOM 1

//global variables
Entity camera;
Entity timer;


PrototypeGame::PrototypeGame() : Game() {
   state2manager.insert(std::make_pair(State::Logo, new LogoStateManager(this)));
   state2manager.insert(std::make_pair(State::Menu, new MenuStateManager(this)));
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theRenderingSystem.loadAtlas("font", true);
    // init font
    loadFont(gameThreadContext->assetAPI, "typo");
    std::list<std::string> files = gameThreadContext->assetAPI->listContent(".atlas");
    for(auto it=files.begin(); it!=files.end(); ++it)
        std::cout << *it << std::endl;
}

void PrototypeGame::init(const uint8_t*, int) {
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


	timer = theEntityManager.CreateEntity("timer");
    ADD_COMPONENT(timer, Transformation);
    TRANSFORM(timer)->z = .9;
    TRANSFORM(timer)->position = glm::vec2(9., -5);
    ADD_COMPONENT(timer, TextRendering);
	TEXT_RENDERING(timer)->show = true;
	TEXT_RENDERING(timer)->text = "0";
	TEXT_RENDERING(timer)->charHeight = 2;
	TEXT_RENDERING(timer)->cameraBitMask = 0xffff;
	TEXT_RENDERING(timer)->positioning = TextRenderingComponent::RIGHT;

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

	//update the timer
    {
    static float timeElapsed = 0.f;
    timeElapsed += dt;

    //update the text from the entity
    std::stringstream a;
    a << gameThreadContext->localizeAPI->text("time") << ": " <<
        std::fixed << std::setprecision(2) << timeElapsed << " s";
    TEXT_RENDERING(timer)->text = a.str();
    }

    {
        //static int i=0;
        //std::cout << "Nombre d'entité = " << ++i << std::endl;

        Entity eq = theEntityManager.CreateEntity();
        ADD_COMPONENT(eq, Transformation);
        TRANSFORM(eq)->z = 0.5;
        TRANSFORM(eq)->size = glm::vec2(0.5,0.5);
        TRANSFORM(eq)->position = glm::vec2(glm::linearRand(-10.0f, 10.0f), glm::linearRand(-10.0f, 10.0f));
        ADD_COMPONENT(eq, Rendering);
        RENDERING(eq)->color = Color::random();
        RENDERING(eq)->show = true;
        RENDERING(eq)->cameraBitMask = 0xffff;
        ADD_COMPONENT(eq, Physics);
        PHYSICS(eq)->mass = 1;
        PHYSICS(eq)->gravity = glm::vec2(0, -1);

		ADD_COMPONENT(eq, AutoDestroy);
        AUTO_DESTROY(eq)->type = AutoDestroyComponent::OUT_OF_AREA;
        AUTO_DESTROY(eq)->params.area.x = AUTO_DESTROY(eq)->params.area.y = 0;
        AUTO_DESTROY(eq)->params.area.w = TRANSFORM(camera)->size.x;
        AUTO_DESTROY(eq)->params.area.h = TRANSFORM(camera)->size.y;
    }
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
