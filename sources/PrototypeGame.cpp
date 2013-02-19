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

#include <base/TouchInputManager.h>
#include <base/MathUtil.h>
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
#include "api/NetworkAPI.h"

#include <cmath>
#include <GL/glfw.h>

PrototypeGame::PrototypeGame(AssetAPI* ast) : Game() {
    asset = ast;

   state2manager.insert(std::make_pair(State::Logo, new LogoStateManager(this)));
   state2manager.insert(std::make_pair(State::Menu, new MenuStateManager(this)));
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theRenderingSystem.loadAtlas("alphabet", true);
    theRenderingSystem.loadAtlas("logo", false);
    theRenderingSystem.loadAtlas("default", false);

    theRenderingSystem.createFramebuffer("pip_camera", 64, 64);
    
    // init font
    loadFont(asset, "typo");
    theRenderingSystem.loadEffectFile("randomize.fs");
}

Entity camera, camera2, pip, hero;
void PrototypeGame::init(const uint8_t*, int) {
    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->setup();
    }

    overrideNextState = State::Invalid;
    currentState = State::Menu;

    std::string runAnim[] = {"soldier_1", "soldier_2", "soldier_1", "soldier_3"};
    theAnimationSystem.registerAnim("hero_run", 
        runAnim,
        4, 10, Interval<int> (-1, -1), "", Interval<float> (0,0));
    theAnimationSystem.registerAnim("hero_idle", 
        runAnim,
        1, 1, Interval<int> (-1, -1), "", Interval<float> (0,0));

    // create hero
    hero = theEntityManager.CreateEntity("local_hero");
    ADD_COMPONENT(hero, Transformation);
    TRANSFORM(hero)->size = Vector2(1.1, 1.7) * 1;
    TRANSFORM(hero)->position = Vector2::Zero;
    TRANSFORM(hero)->z = 0.5;
    ADD_COMPONENT(hero, Rendering);
    RENDERING(hero)->hide = false;
    ADD_COMPONENT(hero, Animation);
    ANIMATION(hero)->name = "hero_idle";
    if (theNetworkSystem.networkAPI && theNetworkSystem.networkAPI->isConnectedToAnotherPlayer()) {
        ADD_COMPONENT(hero, Network);
        NETWORK(hero)->systemUpdatePeriod.insert(std::make_pair(theTransformationSystem.getName(), 0.01));
        NETWORK(hero)->systemUpdatePeriod.insert(std::make_pair(theRenderingSystem.getName(), 0.01));
    }

    // default camera
    camera = theEntityManager.CreateEntity("camera1");
    ADD_COMPONENT(camera, Transformation);
    TRANSFORM(camera)->size = Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH);
    TRANSFORM(camera)->position = Vector2(0, theRenderingSystem.screenH * .3);
    TRANSFORM(camera)->parent = hero;
    ADD_COMPONENT(camera, Camera);
    CAMERA(camera)->enable = true;
    CAMERA(camera)->order = 2;
    CAMERA(camera)->id = 0;
    CAMERA(camera)->clearColor = Color(0.3, 0.1, 0.4);

    // random background
    Entity bg = theEntityManager.CreateEntity("random_bg");
    ADD_COMPONENT(bg, Transformation);
    TRANSFORM(bg)->size = Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH) * 3;
    TRANSFORM(bg)->position = Vector2::Zero;
    TRANSFORM(bg)->z = 0.01;
    ADD_COMPONENT(bg, Rendering);
    RENDERING(bg)->hide = false;
    RENDERING(bg)->texture = theRenderingSystem.loadTextureFile("random_bg");



#if 0
    // PIP camera
    camera2 = theEntityManager.CreateEntity("camera2");
    ADD_COMPONENT(camera2, Transformation);
    TRANSFORM(camera2)->size = Vector2(theRenderingSystem.screenH, theRenderingSystem.screenH);
    TRANSFORM(camera2)->position = Vector2::Zero;
    ADD_COMPONENT(camera2, Camera);
    CAMERA(camera2)->enable = true;
    CAMERA(camera2)->order = 1;
    CAMERA(camera2)->fb = theRenderingSystem.getFramebuffer("pip_camera");
    CAMERA(camera2)->id = 1;
    CAMERA(camera2)->clearColor = Color();
    // to test failure RENDERING(camera2)->hide = false;

    // PIP renderer
    pip = theEntityManager.CreateEntity("pip_renderer");
    ADD_COMPONENT(pip, Transformation);
    TRANSFORM(pip)->size = Vector2(6, 6);
    TRANSFORM(pip)->position = Vector2(-3, 3);
    TRANSFORM(pip)->z = 0.9;
    ADD_COMPONENT(pip, Rendering);
    RENDERING(pip)->hide = false;
    RENDERING(pip)->framebuffer = theRenderingSystem.getFramebuffer("pip_camera");
    RENDERING(pip)->fbo = true;
    RENDERING(pip)->effectRef = theRenderingSystem.loadEffectFile("randomize.fs");
    RENDERING(pip)->cameraBitMask = 0x1;
    ADD_COMPONENT(pip, Particule);
#endif

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
    //TRANSFORM(camera)->rotation += 1. * dt;
    if (overrideNextState != State::Invalid) {
        changeState(overrideNextState);
        overrideNextState = State::Invalid;
    }

    const float speed = 5;
    TransformationComponent* tr = TRANSFORM(hero);
    Vector2 move;
    // move little guy
    if (glfwGetKey( 'Z')) {
        move += Vector2::Rotate(Vector2(0, 1), tr->worldRotation);
    } else if (glfwGetKey('S')) {
        move += Vector2::Rotate(Vector2(0, -0.5), tr->worldRotation);
    }
    if (glfwGetKey ('Q')) {
        move += Vector2::Rotate(Vector2(-1, 0), tr->worldRotation);
    } else if (glfwGetKey ('D')) {
        move += Vector2::Rotate(Vector2(1, 0), tr->worldRotation);
    }
    if (move.X || move.Y) {
        move.Normalize();
        move *= speed * dt;
        tr->position += move;
        ANIMATION(hero)->name = "hero_run";
    } else {
        ANIMATION(hero)->name = "hero_idle";
    }
    const float rotSpeed = 3;
    if (glfwGetKey ('A')) {
        tr->rotation += rotSpeed * dt;
    } else if (glfwGetKey ('E')) {
        tr->rotation -= rotSpeed * dt;
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
    #if 0
    { 
        LOG_EVERY_N(INFO, 50) << "Nouvelle entité";
        static float accum = 0;
        accum += dt * 2;
        while (accum > 1) {
            Entity eq = theEntityManager.CreateEntity("dummy");
            ADD_COMPONENT(eq, Transformation);
            TRANSFORM(eq)->z = 0.5;
            TRANSFORM(eq)->size = Vector2(1, 1) * MathUtil::RandomFloatInRange(0.5, 1.);
            TRANSFORM(eq)->position = Vector2(MathUtil::RandomFloatInRange(-10, 10), MathUtil::RandomFloatInRange(-10, 10));
            ADD_COMPONENT(eq, Rendering);
            RENDERING(eq)->color = Color::random();
            RENDERING(eq)->hide = false;
            RENDERING(eq)->cameraBitMask = 0xffff;
            ADD_COMPONENT(eq, Physics);
            PHYSICS(eq)->mass = MathUtil::RandomFloat();
            PHYSICS(eq)->gravity = Vector2(0, -0.1);
            accum -= 1;
            ADD_COMPONENT(eq, ADSR);
            ADD_COMPONENT(eq, Animation);
            ADD_COMPONENT(eq, AutoDestroy);
            AUTO_DESTROY(eq)->type = AutoDestroyComponent::OUT_OF_SCREEN;
        }
    }
    #endif
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
