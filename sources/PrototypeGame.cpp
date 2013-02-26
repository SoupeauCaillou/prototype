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
    theRenderingSystem.loadAtlas("soccerman", false);

    theRenderingSystem.createFramebuffer("pip_camera", 64, 64);
    
    // init font
    loadFont(asset, "typo");
    theRenderingSystem.loadEffectFile("randomize.fs");
}

Entity camera;
Entity playingField, ball, player;
void PrototypeGame::init(const uint8_t*, int) {
    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->setup();
    }

    overrideNextState = State::Invalid;
    currentState = State::Menu;

    const std::string runAnims[] = {
        "S2", "S1", "S3", "SE2", "SE1", "SE3", "SW2", "SW1", "SW3",
        "E2", "E1", "E3", "W2", "W1", "W3",
        "N2", "N1", "N3", "NE2", "NE1", "NE3", "NW2", "NW1", "NW3",
    };
    const std::string idleAnims[] = {
        "S1", "SE1", "SW1",
        "E1", "W1",
        "N1", "NE1", "NW1"
    };
    const int runAnimFrameCount = 3;
    const float runAnimPlaybackSpeed = 9;
    const Interval<int> noLoop (-1, -1);
    const Interval<float> noWait (0, 0);

    std::string directions[] = {"S", "SE", "SW", "E", "W", "N", "NE", "NW"};
    for (int i=0; i<8; i++) {
        std::stringstream runName, idleName;
        runName << "run" << directions[i];
        theAnimationSystem.registerAnim(runName.str(), &runAnims[i*runAnimFrameCount], runAnimFrameCount, runAnimPlaybackSpeed, noLoop, "", noWait);
        idleName << "idle" << directions[i];
        theAnimationSystem.registerAnim(idleName.str(), &idleAnims[i], 1, runAnimPlaybackSpeed, noLoop, "", noWait);
    }

    // create player
    player = theEntityManager.CreateEntity("player");
    ADD_COMPONENT(player, Transformation);
    TRANSFORM(player)->size = Vector2(1., 1.) * 1.5;
    TRANSFORM(player)->position = Vector2::Zero;
    TRANSFORM(player)->z = 0.5;
    ADD_COMPONENT(player, Rendering);
    RENDERING(player)->hide = false;
    ADD_COMPONENT(player, Animation);
    ADD_COMPONENT(player, Physics);
    PHYSICS(player)->gravity = Vector2::Zero;
    PHYSICS(player)->mass = 1;

    if (theNetworkSystem.networkAPI && theNetworkSystem.networkAPI->isConnectedToAnotherPlayer()) {
        ADD_COMPONENT(player, Network);
        NETWORK(player)->systemUpdatePeriod.insert(std::make_pair(theTransformationSystem.getName(), 0.01));
        NETWORK(player)->systemUpdatePeriod.insert(std::make_pair(theRenderingSystem.getName(), 0.01));
    }

    ball = theEntityManager.CreateEntity("ball");
    ADD_COMPONENT(ball, Transformation);
    TRANSFORM(ball)->size = Vector2(0.4);
    TRANSFORM(ball)->position = Vector2::Zero;
    TRANSFORM(ball)->z = 0.1;
    ADD_COMPONENT(ball, Rendering);
    RENDERING(ball)->hide = false;
    ADD_COMPONENT(ball, Physics);
    PHYSICS(ball)->gravity = Vector2::Zero;
    PHYSICS(ball)->mass = 1;

    // default camera
    camera = theEntityManager.CreateEntity("camera1");
    ADD_COMPONENT(camera, Transformation);
    TRANSFORM(camera)->size = Vector2(theRenderingSystem.screenW * 3, theRenderingSystem.screenH * 3);
    TRANSFORM(camera)->position = Vector2(0, 0);
    // TRANSFORM(camera)->parent = ball;
    ADD_COMPONENT(camera, Camera);
    CAMERA(camera)->enable = true;
    CAMERA(camera)->order = 2;
    CAMERA(camera)->id = 0;
    CAMERA(camera)->clearColor = Color(125.0/255, 150./255.0, 0.);

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

static std::string directionToAnimName(const std::string& prefix, const Vector2& direction) {
    const std::string directions[] = {"E", "NE", "N", "NW", "W", "SW", "S", "SE"};
    float angle = MathUtil::AngleFromVector(direction);
    while (angle < 0) angle += MathUtil::TwoPi;
    while (angle > MathUtil::TwoPi) angle -= MathUtil::TwoPi;
    LOG_IF(FATAL, angle < 0 || angle > MathUtil::TwoPi) << "Invalid angle value: " << angle << ", from direction: " << direction;
    float angle2 = (angle + MathUtil::PiOver4 * 0.5) / MathUtil::PiOver4;
    return prefix + directions[((int)angle2) % 8];
}

bool ballOwner = false;
void PrototypeGame::tick(float dt) {
    //TRANSFORM(camera)->rotation += 1. * dt;
    if (overrideNextState != State::Invalid) {
        changeState(overrideNextState);
        overrideNextState = State::Invalid;
    }

    const float speed = 8;
    const float accel = 80 * dt;
    Vector2& velocity = PHYSICS(player)->linearVelocity;

    bool ballContact = IntersectionUtil::rectangleRectangle(TRANSFORM(player), TRANSFORM(ball));

    // we want to head to the ball if it's near enough
    Vector2 toBall = TRANSFORM(ball)->worldPosition - TRANSFORM(player)->worldPosition;
    float dist = toBall.Normalize();
    //if (ballOwner && dist > 2) ballOwner = false;
    if (glfwGetKey( 'A')) ballOwner = !ballOwner;

    Vector2 moveTarget(Vector2::Zero);

    float weightDirChange = 0.85;
    bool nokeyPressed = true;
    if (!ballOwner || ballContact) {
        // move little guy
        if (glfwGetKey( 'Z')) {
            moveTarget.Y = 1;
            // velocity += Vector2(0, accel) * weightDirChange;
            nokeyPressed = false;
        } else if (glfwGetKey('S')) {
            // velocity += Vector2(0, -accel) * weightDirChange;
            moveTarget.Y = -1;
            nokeyPressed = false;
        }
        if (glfwGetKey ('Q')) {
            moveTarget.X = -1;
            // velocity += Vector2(-accel, 0) * weightDirChange;
            nokeyPressed = false;
        } else if (glfwGetKey ('D')) {
            moveTarget.X = 1;
            // velocity += Vector2(accel, 0) * weightDirChange;
            nokeyPressed = false;
        }
    } else if (ballOwner) {
        nokeyPressed = false;
        const float epsilon = 0.5;
        if (dist > epsilon) {
            moveTarget = toBall;
            // velocity += toBall * accel * weightDirChange;
        }
    }
    if (nokeyPressed) {
        velocity -= velocity * 20 * dt;
        if (velocity.Length() < 0.1) {
            ANIMATION(player)->name = directionToAnimName("idle", velocity);
        }
    } else {
        velocity += moveTarget * (accel * weightDirChange);
        float length = velocity.Normalize();
        if (length > speed) length = speed;
        velocity *= length;
        ANIMATION(player)->name = directionToAnimName("run", velocity);
    }

    // kick ball
    if (!nokeyPressed && ballContact) {
        if (true || Vector2::Dot(PHYSICS(player)->linearVelocity, PHYSICS(ball)->linearVelocity) <= 0) {
            LOG(INFO) << "Kick !";
            const float maxForce = 1000;
            ballOwner = true;
            Vector2 force = moveTarget * maxForce;//PHYSICS(player)->linearVelocity * 100;
            if (force.Length () > maxForce) {
                force.Normalize();
                force *= maxForce;
            }
            PHYSICS(ball)->forces.push_back(std::make_pair(Force(force, Vector2::Zero), 0.016));
        }
    }
    // add friction to ball
    if (PHYSICS(ball)->linearVelocity.LengthSquared() > 0) {
        const float friction = -10;
        PHYSICS(ball)->forces.push_back(std::make_pair(Force(PHYSICS(ball)->linearVelocity * friction, Vector2::Zero), dt));
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
            AUTO_DESTROY(eq)->type = AutoDestroyComponent::OUT_OF_AREA;
            AUTO_DESTROY(eq)->params.area.w = 6;
            AUTO_DESTROY(eq)->params.area.h = 6;
        }
    }
    #endif
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
