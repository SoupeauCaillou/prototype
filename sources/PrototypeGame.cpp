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
#include "systems/AutonomousAgentSystem.h"

#include <cmath>

static Entity e, cursor, wander;

static void updateFps(float dt);

PrototypeGame::PrototypeGame(AssetAPI* ast, NameInputAPI* inputUI, LocalizeAPI* lAPI, AdAPI* ad, ExitAPI* exAPI) : Game() {
	asset = ast;
	exitAPI = exAPI;
}
void PrototypeGame::init(const uint8_t* in, int size) {    
	theRenderingSystem.loadAtlas("alphabet", true);   

	// init font
	loadFont(asset, "typo");
	
	PlacementHelper::GimpWidth = 0; // TODO
    PlacementHelper::GimpHeight = 0; // TODO
    
    cursor = theEntityManager.CreateEntity();
    ADD_COMPONENT(cursor, Transformation);
    TRANSFORM(cursor)->size = Vector2(0.2, 0.2);
    ADD_COMPONENT(cursor, Rendering);
    RENDERING(cursor)->hide = false;
 
    wander = theEntityManager.CreateEntity();
    ADD_COMPONENT(wander, Transformation);
    TRANSFORM(wander)->size = Vector2(0.1, 0.1);
    ADD_COMPONENT(wander, Rendering);
    RENDERING(wander)->color = Color(0.5, 0.5, 0.8);
    RENDERING(wander)->hide = false;
       
    e = theEntityManager.CreateEntity();
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->size = Vector2(0.2, 0.2);
    ADD_COMPONENT(e, Rendering);
    RENDERING(e)->hide = false;
    ADD_COMPONENT(e, Physics);
    PHYSICS(e)->mass = 1;
    ADD_COMPONENT(e, AutonomousAgent);
    AUTONOMOUS_AGENT(e)->maxSpeed = 2;
    AUTONOMOUS_AGENT(e)->maxForce = 100;
    AUTONOMOUS_AGENT(e)->seekTarget = 0;
    AUTONOMOUS_AGENT(e)->seekWeight = 10;
    AUTONOMOUS_AGENT(e)->fleeTarget = 0;
    AUTONOMOUS_AGENT(e)->fleeWeight = 10;
    AUTONOMOUS_AGENT(e)->fleeRadius = 1;
    AUTONOMOUS_AGENT(e)->arriveDeceleration = 0.3;
	AUTONOMOUS_AGENT(e)->wander.radius = 0.5;
	AUTONOMOUS_AGENT(e)->wander.distance = 1;
	AUTONOMOUS_AGENT(e)->wander.jitter = 0.1;
	AUTONOMOUS_AGENT(e)->wander.target = Vector2::UnitX;
}


void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool activate) {

}

void PrototypeGame::tick(float dt) {
	theTouchInputManager.Update(dt);
	
	if (theTouchInputManager.isTouched()) {
		TRANSFORM(cursor)->position = theTouchInputManager.getTouchLastPosition();
		
		RENDERING(cursor)->color = Color(0, 1, 0);
		AUTONOMOUS_AGENT(e)->seekTarget = cursor;
		AUTONOMOUS_AGENT(e)->fleeTarget = 0;
		AUTONOMOUS_AGENT(e)->wanderWeight = 0;
	} else {
		RENDERING(cursor)->color = Color(1, 0, 0);
		AUTONOMOUS_AGENT(e)->seekTarget = 0;
		AUTONOMOUS_AGENT(e)->fleeTarget = cursor;
		AUTONOMOUS_AGENT(e)->wanderWeight = 10;
		TRANSFORM(wander)->position = AUTONOMOUS_AGENT(e)->wander.debugTarget;
	}

    // systems update
	theADSRSystem.Update(dt);
	theButtonSystem.Update(dt);
    theParticuleSystem.Update(dt);
	theMorphingSystem.Update(dt);
	thePhysicsSystem.Update(dt);
	theScrollingSystem.Update(dt);
	theContainerSystem.Update(dt);
	theTextRenderingSystem.Update(dt);
	theAutonomousAgentSystem.Update(dt);
	theSoundSystem.Update(dt);
    theMusicSystem.Update(dt);
    theTransformationSystem.Update(dt);
    theRenderingSystem.Update(dt);
}

void updateFps(float dt) {
    #define COUNT 250
    static int frameCount = 0;
    static float accum = 0, t = 0;
    frameCount++;
    accum += dt;
    if (frameCount == COUNT) {
         LOGI("%d frames: %.3f s - diff: %.3f s - ms per frame: %.3f", COUNT, accum, TimeUtil::getTime() - t, accum / COUNT);
         t = TimeUtil::getTime();
         accum = 0;
         frameCount = 0;
     }
}

