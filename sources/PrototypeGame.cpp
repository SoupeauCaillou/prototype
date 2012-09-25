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
#include <iomanip>
#include <cmath>
#include <vector>

#ifndef EMSCRIPTEN
#include <GL/glfw.h>
#endif

std::vector<Entity> obstacles;
Entity depart, arrivee, player, startButton, chrono;
float startTime;
bool playing;
int obstacleCount = 30;

static void updateFps(float dt);

PrototypeGame::PrototypeGame(AssetAPI* ast, NameInputAPI* inputUI, LocalizeAPI* lAPI, AdAPI* ad, ExitAPI* exAPI) : Game() {
	asset = ast;
	exitAPI = exAPI;
}

static void resetGame() {
    TEXT_RENDERING(startButton)->hide = false;
    CONTAINER(startButton)->enable = true;
    RENDERING(startButton)->hide = false;
    BUTTON(startButton)->enabled = true;
    // RENDERING(player)->hide = true;
    playing = false;
}

static bool collideWithAnotherObstacle(TransformationComponent* tc) {
    for (std::vector<Entity>::iterator it=obstacles.begin(); it!=obstacles.end(); ++it) {
        TransformationComponent* tc2 = TRANSFORM(*it);
        if (IntersectionUtil::rectangleRectangle(tc->position, tc->size * 1.4, tc->rotation,
            tc2->position, tc2->size * 1.4, tc2->rotation)) {
            return true;
        }
    }
    return false;
}

static void initGame() {
    for (int i=0; i<obstacles.size(); i++) {
        theEntityManager.DeleteEntity(obstacles[i]);
    }
    obstacles.clear();

    float minX = 0, maxX = 0;
    for (int i=0; i<obstacleCount; i++) {
        Entity e = theEntityManager.CreateEntity();
        ADD_COMPONENT(e, Transformation);
        TransformationComponent* tc = TRANSFORM(e);
        do {
            tc->size = Vector2(1.7, 0.4);
            tc->position = Vector2(
                (MathUtil::RandomFloat() -0.5 )* PlacementHelper::ScreenWidth,
                (MathUtil::RandomFloat() -0.5 )* PlacementHelper::ScreenHeight);
            tc->rotation = MathUtil::RandomFloat() * 6.28;
        } while (collideWithAnotherObstacle(tc));

        TRANSFORM(e)->z = 0.5;
        ADD_COMPONENT(e, Rendering);
        RENDERING(e)->color = Color::random();
        RENDERING(e)->color.a = 0.5;
        RENDERING(e)->hide = false;
        obstacles.push_back(e);
        
        if (TRANSFORM(e)->position.X < minX) {
            depart = e;
            minX = TRANSFORM(e)->position.X;
        }
        else if (TRANSFORM(e)->position.X > maxX) {
            arrivee = e;
            maxX = TRANSFORM(e)->position.X;
        }
    }
    TRANSFORM(depart)->size = TRANSFORM(arrivee)->size = Vector2(1,1);
    RENDERING(depart)->color = Color(0, 1, 0);
    RENDERING(arrivee)->color = Color(1, 0, 0);
    
    TRANSFORM(player)->position = TRANSFORM(depart)->position;
    TEXT_RENDERING(startButton)->hide = true;
    CONTAINER(startButton)->enable = false;
    RENDERING(startButton)->hide = true;
    BUTTON(startButton)->enabled = false;
    RENDERING(player)->hide = false;
    TEXT_RENDERING(chrono)->hide = false;
    
    ADD_COMPONENT(obstacles[0], ADSR);
    ADSR(obstacles[0])->idleValue = 0;
    ADSR(obstacles[0])->attackValue = 1;
    ADSR(obstacles[0])->attackTiming = 1;
    ADSR(obstacles[0])->sustainValue = 1;
    ADSR(obstacles[0])->decayTiming = 0;
    ADSR(obstacles[0])->releaseTiming = 0;
    
    startTime = TimeUtil::getTime();
    playing = true;
}

void PrototypeGame::init(const uint8_t* in, int size) {
	theRenderingSystem.loadAtlas("alphabet", true);

	// init font
	loadFont(asset, "typo");
	
	PlacementHelper::GimpWidth = 640;
    PlacementHelper::GimpHeight = 480;

    Entity bg = theEntityManager.CreateEntity();
    ADD_COMPONENT(bg, Transformation);
    TRANSFORM(bg)->size = Vector2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
    TRANSFORM(bg)->z = 0.05;
    ADD_COMPONENT(bg, Rendering);
    RENDERING(bg)->hide = false;
    RENDERING(bg)->color = Color(0.5, 0.5, 0.5);
    
    player = theEntityManager.CreateEntity();
    ADD_COMPONENT(player, Transformation);
    TRANSFORM(player)->size = Vector2(0.5,0.5);
    TRANSFORM(player)->z = 0.7;
    ADD_COMPONENT(player, Rendering);
    
    chrono = theEntityManager.CreateEntity();
    ADD_COMPONENT(chrono, Transformation);
    TRANSFORM(chrono)->position = Vector2(0, PlacementHelper::GimpYToScreen(30));
    TRANSFORM(chrono)->z = 0.8;
    ADD_COMPONENT(chrono, TextRendering);
    TEXT_RENDERING(chrono)->charHeight = 1;
    TEXT_RENDERING(chrono)->positioning = TextRenderingComponent::CENTER;

    startButton = theEntityManager.CreateEntity();
    ADD_COMPONENT(startButton, Transformation);
    TRANSFORM(startButton)->position = Vector2::Zero;
    TRANSFORM(startButton)->z = 0.9;
    ADD_COMPONENT(startButton, TextRendering);
    TEXT_RENDERING(startButton)->text = "Jouer";
    TEXT_RENDERING(startButton)->charHeight = 1;
    ADD_COMPONENT(startButton, Container);
    CONTAINER(startButton)->entities.push_back(startButton);
    CONTAINER(startButton)->includeChildren = true;
    ADD_COMPONENT(startButton, Rendering);
    RENDERING(startButton)->color = Color(0.2, 0.2, 0.2, 0.5);
    ADD_COMPONENT(startButton, Button);
    
    resetGame();
}


void PrototypeGame::backPressed() {
#if 0
#ifndef EMSCRIPTEN
    Entity e = theEntityManager.CreateEntity();
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->size = Vector2(1, 1);
    TRANSFORM(e)->z = 0.5;
    ADD_COMPONENT(e, Rendering);
    RENDERING(e)->color = Color::random();
    RENDERING(e)->color.a = 0.5;
    RENDERING(e)->hide = false;
    
    decors.push_back(e);
    activeIndex = decors.size() - 1;
#endif
#endif
}

void PrototypeGame::togglePause(bool activate) {
#if 0
#ifndef EMSCRIPTEN
    if (activeIndex >= 0) {
        Entity e = decors[activeIndex];
        std::cout << "{ " << TRANSFORM(e)->position << ", " 
            << TRANSFORM(e)->size << ", " << TRANSFORM(e)->rotation << "}, " << std::endl;
        activeIndex = -1;
    }
#endif
#endif
}

void PrototypeGame::tick(float dt) {
	theTouchInputManager.Update(dt);
#if 0
#ifndef EMSCRIPTEN
    if (activeIndex >= 0) {
        Entity e = decors[activeIndex];
        if (theTouchInputManager.isTouched()) {
            TRANSFORM(e)->position = theTouchInputManager.getTouchLastPosition();
        }
            
        // mouse wheel -> rotate
        {
            static int prevWheel = 0;
            int wheel = glfwGetMouseWheel();
            int diff = wheel - prevWheel;
            if (diff) {
                bool shift = glfwGetKey( GLFW_KEY_LSHIFT );
                bool ctrl = glfwGetKey( GLFW_KEY_LCTRL );
                
                if (!shift && !ctrl) {
                    TRANSFORM(e)->rotation += 2 * diff * dt;
                } else {
                    if (shift) {
                        TRANSFORM(e)->size.X *= (1 + 1 * diff * dt); 
                    }
                    if (ctrl) {
                        TRANSFORM(e)->size.Y *= (1 + 1 * diff * dt); 
                    }
                }
                prevWheel = wheel;
            }
        }
    } else if (theTouchInputManager.isTouched()) {
        for (int i=0; i<decors.size(); i++) {
            if (IntersectionUtil::pointRectangle(
                theTouchInputManager.getTouchLastPosition(), 
                TRANSFORM(decors[i])->position,
                TRANSFORM(decors[i])->size)) {
                activeIndex = i;
                break;
            }
        }
    }
#endif
#endif

    if (BUTTON(startButton)->clicked) {
        std::cout << "Click" << std::endl;
        initGame();
    } else {
        if (playing) {
            TransformationComponent* tc = TRANSFORM(player);
            if (theTouchInputManager.isTouched()) {
                const Vector2& newPos = theTouchInputManager.getTouchLastPosition();
                if (Vector2::Distance(newPos, tc->position) <= tc->size.X) {
                    tc->position = theTouchInputManager.getTouchLastPosition();
                }
                ADSR(obstacles[0])->active = true;
            }
            
            float dt = TimeUtil::getTime() - startTime;
            std::stringstream a;
            a << std::fixed << std::setprecision(1) << dt << " s";
            TEXT_RENDERING(chrono)->text = a.str();
            
            float alpha = 1 - ADSR(obstacles[0])->value;
            for (std::vector<Entity>::iterator it=obstacles.begin(); it!=obstacles.end(); ++it) {
                Entity e = *it;
                if (e != arrivee && e != depart) {
                    RENDERING(*it)->color.a = alpha;
                    
                    if (IntersectionUtil::rectangleRectangle(tc, TRANSFORM(*it))) {
                        RENDERING(e)->color.a = 1;
                        TEXT_RENDERING(startButton)->text = "Perdu. Rejouer";
                        obstacleCount -= 10;
                        resetGame();
                    }
                } else if (e == arrivee) {
                    if (IntersectionUtil::rectangleRectangle(tc, TRANSFORM(*it)) ){
                        std::cout << "Gagné" << std::endl;
                        TEXT_RENDERING(startButton)->text = "Gagné. Rejouer";
                        obstacleCount += 10;
                        resetGame();
                    }
                }
            }
        }
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

