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
#include "systems/GraphSystem.h"
#include "api/NetworkAPI.h"
#include "FieldPlayerSystem.h"
#include "BallSystem.h"
#include "AISystem.h"
#include "TeamSystem.h"

#include <cmath>
#include <GL/glfw.h>

#define ZOOM 2

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

Entity camera;
Entity playingField, ball, teams[2];
std::vector<Entity> players;
unsigned activePlayer = 0;
ImageDesc desc;
float offset;
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

int count = 0;
float accum = 0, accum2=0;
bool playerSwitchDown = false;
void PrototypeGame::tick(float dt) {
    accum += 5 * dt;

#if 0
    while (0 && accum > 1) {
        accum -=1;
        count++;
        float r = -1 + 2 * MathUtil::RandomFloat();
        GRAPH(ball)->pointsList.push_back(std::make_pair(count++, r * r));
    }
    while (GRAPH(ball)->pointsList.size() > 100) GRAPH(ball)->pointsList.pop_front();
    accum2 += dt;
    GRAPH(playingField)->pointsList.push_back(std::make_pair(accum2, cos(accum2)));
    while (GRAPH(playingField)->pointsList.size() > 250) GRAPH(playingField)->pointsList.pop_front();
    GRAPH(ball)->reloadTexture = true;
#endif
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
    /*
    GRAPH_SYSTEM(ball)->pointsList.push_back(
        std::make_pair(count++, dt));
    while (GRAPH_SYSTEM(ball)->pointsList.size() > 300)
        GRAPH_SYSTEM(ball)->pointsList.pop_front();
        */
        
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
