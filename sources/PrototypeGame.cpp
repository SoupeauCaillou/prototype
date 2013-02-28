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
#include "FieldPlayerSystem.h"
#include "BallSystem.h"
#include "AISystem.h"
#include "TeamSystem.h"

#include <cmath>
#include <GL/glfw.h>

#define ZOOM 2

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

    FieldPlayerSystem::CreateInstance();
    BallSystem::CreateInstance();
    AISystem::CreateInstance();
    TeamSystem::CreateInstance();
}

static Entity addPlayer(Entity team, const Vector2& position) {
    Entity player = theEntityManager.CreateEntity("player");
    ADD_COMPONENT(player, Transformation);
    TRANSFORM(player)->size = Vector2(1., 1.) * 1.5;
    TRANSFORM(player)->position = TRANSFORM(player)->worldPosition = position;
    // MathUtil::RandomVector(Vector2(theRenderingSystem.screenW * ZOOM, theRenderingSystem.screenH)) -
    //    Vector2(theRenderingSystem.screenW * ZOOM, theRenderingSystem.screenH) * 0.5;
    TRANSFORM(player)->z = 0.5;
    ADD_COMPONENT(player, Rendering);
    RENDERING(player)->color = TEAM(team)->color;
    RENDERING(player)->hide = false;
    ADD_COMPONENT(player, Animation);
    ADD_COMPONENT(player, Physics);
    PHYSICS(player)->gravity = Vector2::Zero;
    PHYSICS(player)->mass = 1;
    ADD_COMPONENT(player, FieldPlayer);
    Entity contact = theEntityManager.CreateEntity("contact");
    ADD_COMPONENT(contact, Transformation);
    TRANSFORM(contact)->parent = player;
    TRANSFORM(contact)->size = Vector2(0.5, 0.5);
    TRANSFORM(contact)->position = Vector2(0., -0.25);
    ADD_COMPONENT(contact, Rendering); // debug
    RENDERING(contact)->hide = true;
    FIELD_PLAYER(player)->ballContact = contact;
    FIELD_PLAYER(player)->team = team;
    ADD_COMPONENT(player, AI);
    return player;
}

Entity camera;
Entity playingField, ball, teams[2];
std::vector<Entity> players;
unsigned activePlayer = 0;
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

    Vector2 positions[] = {
        Vector2(0, -3),
        Vector2(-12, -7), Vector2(12, -7),
        Vector2(0, -10),
        Vector2(-9, -15), Vector2(9, -15)
    };

    teams[0] = theEntityManager.CreateEntity("team_red");
    ADD_COMPONENT(teams[0], Team);
    TEAM(teams[0])->color = Color(1, 0.5, 0.5);
    teams[1] = theEntityManager.CreateEntity("team_blue");
    ADD_COMPONENT(teams[1], Team);
    TEAM(teams[1])->color = Color(0.5, 0.5, 1);

    ball = theEntityManager.CreateEntity("ball");
    ADD_COMPONENT(ball, Transformation);
    TRANSFORM(ball)->size = Vector2(0.4);
    TRANSFORM(ball)->position = Vector2::Zero;
    TRANSFORM(ball)->z = 0.1;
    ADD_COMPONENT(ball, Rendering);
    RENDERING(ball)->hide = false;
    RENDERING(ball)->texture = theRenderingSystem.loadTextureFile("ballon1");
    ADD_COMPONENT(ball, Physics);
    PHYSICS(ball)->gravity = Vector2::Zero;
    PHYSICS(ball)->mass = 1;
    ADD_COMPONENT(ball, Ball);

    playingField = theEntityManager.CreateEntity("playingField");
    ADD_COMPONENT(playingField, Transformation);
    TRANSFORM(playingField)->size = Vector2(theRenderingSystem.screenW * 2, theRenderingSystem.screenW * 2 * 1.5);
    TRANSFORM(playingField)->position = Vector2::Zero;
    TRANSFORM(playingField)->z = 0.01;
    ADD_COMPONENT(playingField, Rendering);
    RENDERING(playingField)->hide = false;
    RENDERING(playingField)->texture = theRenderingSystem.loadTextureFile("palyingfield");

    // default camera
    camera = theEntityManager.CreateEntity("camera1");
    ADD_COMPONENT(camera, Transformation);
    TRANSFORM(camera)->size = Vector2(theRenderingSystem.screenW * ZOOM, theRenderingSystem.screenH * ZOOM);
    TRANSFORM(camera)->position = Vector2(0, 0);
    //TRANSFORM(camera)->parent = ball;
    ADD_COMPONENT(camera, Camera);
    CAMERA(camera)->enable = true;
    CAMERA(camera)->order = 2;
    CAMERA(camera)->id = 0;
    CAMERA(camera)->clearColor = Color(125.0/255, 150./255.0, 0.);

    // create player
    for (int i=0; i<6; i++)
        for (int j=0; j<2; j++)
            players.push_back(addPlayer(teams[j], positions[i] * Vector2(1, 1 + j * -2)));
#if 0
    if (theNetworkSystem.networkAPI && theNetworkSystem.networkAPI->isConnectedToAnotherPlayer()) {
        ADD_COMPONENT(player, Network);
        NETWORK(player)->systemUpdatePeriod.insert(std::make_pair(theTransformationSystem.getName(), 0.01));
        NETWORK(player)->systemUpdatePeriod.insert(std::make_pair(theRenderingSystem.getName(), 0.01));
    }
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

bool playerSwitchDown = false;
void PrototypeGame::tick(float dt) {
    #ifndef BEPO
    static const char GOforward = 'Z';
    static const char GObackward = 'S';
    static const char GOleft = 'Q';
    static const char GOright = 'D';
    #else
    static const char GOforward = '/';
    static const char GObackward = 'U';
    static const char GOleft = 'A';
    static const char GOright = 'I';
    #endif

    //TRANSFORM(camera)->rotation += 1. * dt;
    if (overrideNextState != State::Invalid) {
        changeState(overrideNextState);
        overrideNextState = State::Invalid;
    }
    Entity player = players[activePlayer];
    Vector2& direction = FIELD_PLAYER(player)->input.direction;
    if (glfwGetKey(GOforward) || glfwGetKey(GLFW_KEY_UP))
        direction.Y = 1;
    else if (glfwGetKey(GObackward) || glfwGetKey(GLFW_KEY_DOWN))
        direction.Y = -1;
    if (glfwGetKey(GOleft) || glfwGetKey(GLFW_KEY_LEFT))
        direction.X = -1;
    else if (glfwGetKey(GOright) || glfwGetKey(GLFW_KEY_RIGHT))
        direction.X = 1;
    if (direction != Vector2::Zero)
        direction.Normalize();

    if (glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT))
        playerSwitchDown = true;
    else if (playerSwitchDown) {
        if (BALL(ball)->owner == player) {
            FIELD_PLAYER(player)->input.action |= PASS;
            LOG(INFO) << "Request pass";
        } else {
            activePlayer = (activePlayer + 1) % players.size();
        }
        playerSwitchDown = false;
    }

    // simple camera tracking
    Entity trackedEntity = BALL(ball)->owner;
    if (!trackedEntity) trackedEntity = ball;
    float yDiff = TRANSFORM(trackedEntity)->worldPosition.Y - TRANSFORM(camera)->worldPosition.Y;
    float tolerance = TRANSFORM(camera)->size.Y * .01;
    if (MathUtil::Abs(yDiff) > tolerance) {
        TRANSFORM(camera)->position.Y += MathUtil::Max(2 * yDiff * dt, PHYSICS(trackedEntity)->linearVelocity.Y * dt);
    }
    // limit cam position
    TRANSFORM(camera)->position.Y = MathUtil::Min(
            (TRANSFORM(playingField)->size.Y - TRANSFORM(camera)->size.Y) * 0.5f,
            MathUtil::Max(TRANSFORM(camera)->position.Y, (-TRANSFORM(playingField)->size.Y + TRANSFORM(camera)->size.Y) * 0.5f));

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

    theTeamSystem.Update(dt);
    theAISystem.Update(dt);
    theFieldPlayerSystem.Update(dt);
    theBallSystem.Update(dt);
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
