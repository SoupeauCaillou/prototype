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
#include "api/AssetAPI.h"

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
    // init font
    loadFont(asset, "typo");
}

void PrototypeGame::init(const uint8_t*, int) {
    for(std::map<State::Enum, StateManager*>::iterator it=state2manager.begin(); it!=state2manager.end(); ++it) {
        it->second->setup();
    }

    overrideNextState = State::Invalid;
    currentState = State::Logo;

    quickInit();

    test = new PixelManager("1.png");
    //~ if (test->changeBackGround("1.png"))
        //~ std::cout << "ok" << std::endl;
    
    //~ Entity eq = theEntityManager.CreateEntity();
    //~ ADD_COMPONENT(eq, Transformation);
    //~ TRANSFORM(eq)->z = 0.1;
    //~ TRANSFORM(eq)->size = Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH);
    //~ TRANSFORM(eq)->position = Vector2(0,0);
    //~ ADD_COMPONENT(eq, Rendering);
//~ 
    //~ // Load an image :
    //~ FileBuffer f;
    //~ switch(MathUtil::RandomIntInRange(1, 3))
    //~ {
        //~ case 1:
            //~ f = asset->loadAsset("1.png");
            //~ break;
        //~ case 2:
            //~ f = asset->loadAsset("2.png");
            //~ break;
        //~ default:
            //~ f = asset->loadAsset("3.png");
    //~ }
//~ 
    //~ if (f.data) {
        //~ bg = ImageLoader::loadPng("Image png", f);
        //~ LOGI("Calcul de la moyenne de l'image :");
        //~ LOGI("Canaux : %d", bg.channels);
        //~ LOGI("Mipmap : %d", bg.mipmap);
        //~ LOGI("Taille : %d x %d", bg.width, bg.height);
        //~ RENDERING(eq)->color = moyennePixel(TRANSFORM(eq)->size, TRANSFORM(eq)->position);
    //~ }
//~ 
    //~ RENDERING(eq)->hide = false;
    //~ e.push_back(eq);
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

    Vector2 p = Vector2(-999, -999);
    if (currentState != State::Logo)
    {
        if (theTouchInputManager.isTouched(0))
        {
            p = theTouchInputManager.getTouchLastPosition(0);
        }
        else
        {
            //~ p = randomSplit();
        }
    }
    if (theTouchInputManager.wasTouched(0) && p != Vector2(-999, -999))
    {
        test->clickedOn(p);
        //~ for (std::list<Entity>::iterator it = e.begin(); it != e.end(); ++it)
        //~ {
            //~ if (entityInMotion.count(*it))
                //~ continue;
            //~ Entity x = *it;
            //~ if (IntersectionUtil::pointRectangle(p, TRANSFORM(x)->position, TRANSFORM(x)->size))
            //~ {
                //~ if (createNewEntity(x))
                //~ {
                    //~ destructParent(x);
                    //~ e.remove(x);
                //~ }
                //~ break;
            //~ }
        //~ }
    }
    if (currentState != State::Logo)
        test->updatePixel();

    //~ if (currentState != State::Logo)
    //~ {
        //~ p = randomFuse();
    //~ }

    //~ if (p != Vector2(-999, -999))
    //~ {
        //~ for (std::list<Entity>::iterator it = e.begin(); it != e.end(); ++it)
        //~ {
            //~ const Entity& x = *it;
            //~ if (entityInMotion.count(x))
                //~ continue;
            //~ if (IntersectionUtil::pointRectangle(p, TRANSFORM(x)->position, TRANSFORM(x)->size))
            //~ {
                //~ fuseEntity(x);
                //~ break;
            //~ }
        //~ }
    //~ }

    //~ if (!entityInMotion.empty())
    //~ {
        //~ moveEntity();
    //~ }
    
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}

bool PrototypeGame::createNewEntity(const Entity &parent)
{
    static int DIVIDE_BY = 4;
    if (!(TRANSFORM(parent)->size < Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH) * 0.02))
    {
        for (int i=0; i<DIVIDE_BY; ++i)
        {
            Entity eq = 0;
            eq = theEntityManager.CreateEntity();
            ADD_COMPONENT(eq, Transformation);
            TRANSFORM(eq)->z = MathUtil::RandomFloatInRange(0.1, 1);
            TRANSFORM(eq)->size = TRANSFORM(parent)->size;
            TRANSFORM(eq)->position = TRANSFORM(parent)->position;
            ADD_COMPONENT(eq, Rendering);
            RENDERING(eq)->color = RENDERING(parent)->color;
            RENDERING(eq)->hide = false;
            e.push_back(eq);
            entityInMotion[eq] = Vector2(TRANSFORM(parent)->position.X + TRANSFORM(parent)->size.X/4 * (-1+2*(i%2)),
                                         TRANSFORM(parent)->position.Y + TRANSFORM(parent)->size.Y/4 * (-1+2*(i/2)));
            //~ colorInMotion[eq] = Color::random();
            
            sizeInMotion[eq] = TRANSFORM(parent)->size / 2;
            colorInMotion[eq] = moyennePixel(sizeInMotion[eq], entityInMotion[eq]);
        }
        return true;
    }
    return false;
}

void PrototypeGame::destructParent(const Entity &parent)
{
    theEntityManager.DeleteEntity(parent);
}

void PrototypeGame::moveEntity()
{
    static float SPEED = 0.05;
    std::map<Entity, Color>::iterator it2=colorInMotion.begin();
    std::map<Entity, Vector2>::iterator it3=sizeInMotion.begin();
    int i=0;
    for (std::map<Entity, Vector2>::iterator it=entityInMotion.begin(); it != entityInMotion.end(); ++it, ++it2, ++it3, ++i)
    {
        if (TRANSFORM(it->first)->position == it->second)
        {
            entityInMotion.erase(it);
            colorInMotion.erase(it2);
            sizeInMotion.erase(it3);
            continue;
        }
        
        Vector2 step = Vector2().Normalize(it->second - TRANSFORM(it->first)->position) * SPEED;
        Vector2 t = it->second - TRANSFORM(it->first)->position;

        Color c = RENDERING(it2->first)->color;

        RENDERING(it2->first)->color.r += (it2->second.r - c.r) * (step.X / t.X);
        RENDERING(it2->first)->color.g += (it2->second.g - c.g) * (step.X / t.X);
        RENDERING(it2->first)->color.b += (it2->second.b - c.b) * (step.X / t.X);

        Vector2 s = TRANSFORM(it3->first)->size;

        TRANSFORM(it3->first)->size +=  (it3->second - s) * (step.X / t.X);

        TRANSFORM(it->first)->position += step;
        
        if (Vector2::Distance(it->second, TRANSFORM(it->first)->position) < 0.05)
        {
            TRANSFORM(it->first)->position = it->second;
            entityInMotion.erase(it);
            RENDERING(it2->first)->color = it2->second;
            colorInMotion.erase(it2);
            TRANSFORM(it3->first)->size = it3->second;
            sizeInMotion.erase(it3);
            
        }
    }
}

Color PrototypeGame::moyennePixel(Vector2 size, Vector2 position)
{
    uint moyR=0, moyG=0, moyB=0;
    //~ Color moy = Color(0,0,0);
    Vector2 sizeInPixel = size;
    sizeInPixel.X = int(bg.width * (sizeInPixel.X /theRenderingSystem.screenW));
    sizeInPixel.Y = int(bg.height * (sizeInPixel.Y /theRenderingSystem.screenH));

    Vector2 pos = (position + (Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH) - size) / 2) / Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH);
    pos *= Vector2(bg.width, bg.height);
    pos.X = int(pos.X);
    pos.Y = int(pos.Y);

    LOGI("Image size : %d x %d", bg.width, bg.height);
    LOGI("Position in image {%f, %f}", pos.X, pos.Y);
    LOGI("Size to average {%f, %f}", sizeInPixel.X, sizeInPixel.Y);

    //~ std::cout << "début = " << pos.X << " fin = " << j << std::endl;
    for (int i=pos.X*bg.channels; i<(pos.X + sizeInPixel.X)*bg.channels; i+=bg.channels)
    {
        for (int j=(bg.height - pos.Y)*bg.channels; j>((bg.height - pos.Y) - sizeInPixel.Y)*bg.channels; j-=bg.channels)
        {
            
            moyR += bg.datas[i + j*bg.width] & 0xFF;
            moyG += bg.datas[i + j*bg.width + 1] & 0xFF;
            moyB += bg.datas[i + j*bg.width + 2] & 0xFF;
        }
    }
    
    moyR /= (sizeInPixel.X * sizeInPixel.Y);
    moyG /= (sizeInPixel.X * sizeInPixel.Y);
    moyB /= (sizeInPixel.X * sizeInPixel.Y);

    LOGI("Value of Red canal in selection : %d", moyR);
    LOGI("Value of Green canal in selection : %d", moyG);
    LOGI("Value of Blue canal in selection : %d", moyB);
    
    return Color(moyR/256.,moyG/256.,moyB/256.);
}

Vector2 PrototypeGame::randomSplit()
{
    if (MathUtil::RandomIntInRange(0, 60) < 1)
    {
        return Vector2(MathUtil::RandomFloatInRange(-10, 10), MathUtil::RandomFloatInRange(-10, 10));
    }

    return Vector2(-999, -999);
}

Vector2 PrototypeGame::randomFuse()
{
    if (MathUtil::RandomIntInRange(0, 60) < 3)
    {
        return Vector2(MathUtil::RandomFloatInRange(-10, 10), MathUtil::RandomFloatInRange(-10, 10));
    }

    return Vector2(-999, -999);
}

bool PrototypeGame::fuseEntity(const Entity &parent)
{
    if (TRANSFORM(parent)->size != Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH))
    {

    }
}
