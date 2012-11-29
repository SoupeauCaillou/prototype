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

#if 0
struct Slider {
    Interval<float> values;
    float position;
    int cursorIndex;
    Vector2 size;
    Entity fg, bg;
};

void createSlider(Slider& s) {
    s.bg = theEntityManager.CreateEntity();
    ADD_COMPONENT(s.bg, Transformation);
    TRANSFORM(s.bg)->size = s.size;
    TRANSFORM(s.bg)->z = 0.9;
    ADD_COMPONENT(s.bg, Rendering);
    RENDERING(s.bg)->color.a = 0.5;
    RENDERING(s.bg)->hide = false;
    
    s.fg = theEntityManager.CreateEntity();
    ADD_COMPONENT(s.fg, Transformation);
    TRANSFORM(s.fg)->parent = s.bg;
    TRANSFORM(s.fg)->z = 0.01;
    TRANSFORM(s.fg)->size = s.size * Vector2(0.95, 0.2);
    ADD_COMPONENT(s.fg, Rendering);
    RENDERING(s.fg)->color.a = 0.5;
    RENDERING(s.fg)->hide = false;
    
    s.cursorIndex = -1;
    s.position = 0;
    TRANSFORM(s.fg)->position.Y = TRANSFORM(s.bg)->size.Y * -0.5 + TRANSFORM(s.fg)->size.Y * 0.5;
}

bool updateSlider(Slider& s) {
    if (s.cursorIndex < 0) {
        for (int i=0; i<3; i++) {
            if (theTouchInputManager.isTouched(i) && 
                IntersectionUtil::pointRectangle(
                    theTouchInputManager.getTouchLastPosition(i),
                    TRANSFORM(s.fg)->worldPosition, TRANSFORM(s.fg)->size)) {
                s.cursorIndex = i;
                break;
             }
        }
    } else {
        if (!theTouchInputManager.isTouched(s.cursorIndex)) {
            s.cursorIndex = -1;
        } else if (!IntersectionUtil::pointRectangle(
                    theTouchInputManager.getTouchLastPosition(s.cursorIndex),
                    TRANSFORM(s.fg)->worldPosition, TRANSFORM(s.fg)->size)) {
            s.cursorIndex = -1;
        }
    }
    if (s.cursorIndex >= 0) {
        const float clickPos = theTouchInputManager.getTouchLastPosition(s.cursorIndex).Y;
        const float p = TRANSFORM(s.bg)->worldPosition.Y;
        s.position = (clickPos - (p - TRANSFORM(s.bg)->size.Y * 0.5)) / TRANSFORM(s.bg)->size.Y;
        s.position = MathUtil::Min(0.9f, MathUtil::Max(s.position, 0.1f));
        TRANSFORM(s.fg)->position.Y = TRANSFORM(s.bg)->size.Y * (s.position - 0.5);
        s.position = s.position * (10.0 / 8) - 0.125;
        return true;
    } else {
        return false;
    }
}
#endif

Entity chassis, element[3], rotationE[3], dummy[3];
// Slider sliders[3];
Entity controls[6];
Entity moveButton[2];

Vector2 sizes[] = {
    Vector2(343, 248),
    Vector2(353, 178),
    Vector2(306, 59),
    Vector2(101, 117)
};
Vector2 positions[] = {
    Vector2(0.7, 0.25),
    Vector2(0.7, 1),
    Vector2(1.6, -0.)
};

Vector2 rotationOrigin[] = {
    Vector2(0.43, 0.35),
    Vector2(0.32, 0.17),
    Vector2(0.4, -0.05)
};

Interval<float> allowedAngles[] = {
    Interval<float>(-0.8, 0.8),
    Interval<float>(-2, 0.2),
    Interval<float>(-2, 1)
};

float rotSpeed[] = {
    1, 1.8, 2
};

static void updateFps(float dt);

PrototypeGame::PrototypeGame(AssetAPI* ast, NameInputAPI* inputUI, LocalizeAPI* lAPI, AdAPI* ad, ExitAPI* exAPI) : Game() {
	asset = ast;
	exitAPI = exAPI;
}
void PrototypeGame::init(const uint8_t* in, int size) {    
	theRenderingSystem.loadAtlas("alphabet", true);
    theRenderingSystem.loadAtlas("pelleteuse", true); 

	// init font
	loadFont(asset, "typo");
	
	PlacementHelper::GimpWidth = 640;
    PlacementHelper::GimpHeight = 480;
    
    Entity bg = theEntityManager.CreateEntity();
    ADD_COMPONENT(bg, Transformation);
    TRANSFORM(bg)->size = Vector2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
    TRANSFORM(bg)->z = 0.01;
    ADD_COMPONENT(bg, Rendering);
    RENDERING(bg)->color = Color(0.8, 0.8, 0.8, 1);
    // RENDERING(bg)->hide = false;
    
    float scale = 0.3;
    chassis = theEntityManager.CreateEntity();
    ADD_COMPONENT(chassis, Transformation);
    TRANSFORM(chassis)->z = 0.5;
    TRANSFORM(chassis)->size = Vector2(PlacementHelper::GimpWidthToScreen(sizes[0].X), PlacementHelper::GimpHeightToScreen(sizes[0].Y)) * scale;
    TRANSFORM(chassis)->position = Vector2(
        PlacementHelper::ScreenWidth * -0.3,
        PlacementHelper::ScreenHeight * 0.1);
    ADD_COMPONENT(chassis, Rendering);
    RENDERING(chassis)->hide = false;
    RENDERING(chassis)->texture = theRenderingSystem.loadTextureFile("chassis");
    

    for (int i=0; i<3; i++) {
        element[i] = theEntityManager.CreateEntity();
        ADD_COMPONENT(element[i], Transformation);
        if (i == 0)
            TRANSFORM(element[i])->parent = chassis;
        else
            TRANSFORM(element[i])->parent = element[i-1];
        TRANSFORM(element[i])->size = Vector2(PlacementHelper::GimpWidthToScreen(sizes[i+1].X), PlacementHelper::GimpHeightToScreen(sizes[i+1].Y)) * scale;
        TRANSFORM(element[i])->position = positions[i] * TRANSFORM(element[i])->size;
        ADD_COMPONENT(element[i], Rendering);
        std::stringstream a;
        a << "element" << i;
        RENDERING(element[i])->texture = theRenderingSystem.loadTextureFile(a.str());
        RENDERING(element[i])->hide = false;
        TRANSFORM(element[i])->z = 0.01 ;
        TRANSFORM(element[i])->localRotationCenter = TRANSFORM(element[i])->size * rotationOrigin[i];

        dummy[i] = theEntityManager.CreateEntity();
        ADD_COMPONENT(dummy[i], Transformation);
        // TRANSFORM(dummy[i])->parent = element[i];
        TRANSFORM(dummy[i])->z = 1;
        TRANSFORM(dummy[i])->size = Vector2(0.5);//Vector2(2 * MathUtil::Max(TRANSFORM(element[i])->size.X, TRANSFORM(element[i])->size.Y));
        ADD_COMPONENT(dummy[i], Rendering);
        // RENDERING(dummy[i])->texture = theRenderingSystem.loadTextureFile("cercle");
        RENDERING(dummy[i])->color = Color(1, 0, 0, 0.5);
        // RENDERING(dummy[i])->hide = false;

        rotationE[i] = theEntityManager.CreateEntity();
        ADD_COMPONENT(rotationE[i], Transformation);
        TRANSFORM(rotationE[i])->parent = element[i];
        TRANSFORM(rotationE[i])->position = -TRANSFORM(element[i])->localRotationCenter;
        TRANSFORM(rotationE[i])->size = Vector2(0.25);
        TRANSFORM(rotationE[i])->z = 0.1;
        ADD_COMPONENT(rotationE[i], Rendering);
        RENDERING(rotationE[i])->color = Color(1,1,1,.5);
        RENDERING(rotationE[i])->hide = false;
        
        for (int j=0; j<2; j++) {
            controls[2*i+j] = theEntityManager.CreateEntity();
            ADD_COMPONENT(controls[2*i+j], Transformation);
            TRANSFORM(controls[2*i+j])->size = Vector2(2, PlacementHelper::ScreenHeight * 0.5);
            TRANSFORM(controls[2*i+j])->position.Y = (j ? 1 : -1) * PlacementHelper::ScreenHeight * 0.25;
            TRANSFORM(controls[2*i+j])->z = 1;
            ADD_COMPONENT(controls[2*i+j], Rendering);
            RENDERING(controls[2*i+j])->color = Color(1,0,j,0.7);
            // RENDERING(controls[2*i+j])->hide = false;
            ADD_COMPONENT(controls[2*i+j], Button);
            BUTTON(controls[2*i+j])->enabled = false;
        }
        
        moveButton[i] = theEntityManager.CreateEntity();
        ADD_COMPONENT(moveButton[i], Transformation);
        TRANSFORM(moveButton[i])->position = Vector2((i ? 1 : -1) * PlacementHelper::ScreenWidth * 0.25, 
            -PlacementHelper::ScreenHeight * 0.5 + 0.5);
        TRANSFORM(moveButton[i])->size = Vector2(PlacementHelper::ScreenWidth * 0.5, 2);
        TRANSFORM(moveButton[i])->z = 1;
        ADD_COMPONENT(moveButton[i], Rendering);
        RENDERING(moveButton[i])->color = Color(i ? 1 : 0, i ? 0:1, 0);
        RENDERING(moveButton[i])->hide = false;
        ADD_COMPONENT(moveButton[i], Button);
        BUTTON(moveButton[i])->enabled = true;
    }
    for (int j=0; j<2; j++) {
        TRANSFORM(controls[0+j])->position.X = -PlacementHelper::ScreenWidth * 0.5 + 1;
        TRANSFORM(controls[2+j])->position.X = PlacementHelper::ScreenWidth * 0.5 - 1;
        TRANSFORM(controls[4+j])->position.X = 0;
        
        RENDERING(controls[4+j])->hide = true;
    }
    
    #if 0
        sliders[i].size = Vector2(1, PlacementHelper::ScreenHeight);
        createSlider(sliders[i]);
    }
    TRANSFORM(sliders[0].bg)->position = Vector2(-PlacementHelper::ScreenWidth * 0.5 + 0.5, 0);
    sliders[0].values = Interval<float>(0, -2);
    TRANSFORM(sliders[1].bg)->position = Vector2(0, 0);
    sliders[1].values = Interval<float>(0, -2);
    TRANSFORM(sliders[2].bg)->position = Vector2(PlacementHelper::ScreenWidth * 0.5 - 0.5, 0);
    sliders[2].values = Interval<float>(0, -2);
    #endif
}


void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool activate) {

}

Vector2 _rA, _rB, rotMark, dragMark, dragSt;
float rotA, rotB, rotC, targetAbsRot;
int which = -1;
void PrototypeGame::tick(float dt) {
	theTouchInputManager.Update(dt);
 if (_rA == Vector2::Zero) {
    _rA = TRANSFORM(rotationE[1])->worldPosition - TRANSFORM(rotationE[0])->worldPosition;
    _rB = TRANSFORM(rotationE[2])->worldPosition - TRANSFORM(rotationE[1])->worldPosition;
 }
 
    if (BUTTON(moveButton[0])->mouseOverIndex >= 0) {
        TRANSFORM(chassis)->position.X -= 3 * dt;
    } else if (BUTTON(moveButton[1])->mouseOverIndex >= 0) {
        TRANSFORM(chassis)->position.X += 3 * dt;
    } else if (theTouchInputManager.isTouched(0)) {
        const Vector2& p = theTouchInputManager.getTouchLastPosition(0);
        
        if (theTouchInputManager.isTouched(1)) {
            if (!theTouchInputManager.wasTouched(1)) {
                rotMark = theTouchInputManager.getTouchLastPosition(1);
                RENDERING(dummy[1])->hide = false;
                rotC = TRANSFORM(element[2])->rotation;
            } else {
                const Vector2& ppp = TRANSFORM(element[2])->worldPosition;
                targetAbsRot -=
                    MathUtil::AngleFromVectors(rotMark - ppp,
                        theTouchInputManager.getTouchLastPosition(1) - ppp);
                rotMark = theTouchInputManager.getTouchLastPosition(1);
            }
            TRANSFORM(dummy[1])->position = rotMark;
        } else {
            RENDERING(dummy[1])->hide = true;
            
            if (!theTouchInputManager.wasTouched(0)) {
                dragMark = p;
                dragSt = TRANSFORM(rotationE[2])->worldPosition;
                targetAbsRot = TRANSFORM(rotationE[2])->worldRotation;
            } else {
                Vector2 target = dragSt + (p - dragMark) * 0.5;
                 float alpha = allowedAngles[0].t1, beta = allowedAngles[1].t1;
                // const float l1 = (TRANSFORM(rotationE[1])->worldPosition - TRANSFORM(rotationE[0])->worldPosition).Length();
                // const float l2 = (TRANSFORM(rotationE[2])->worldPosition - TRANSFORM(rotationE[1])->worldPosition).Length();
                
                // brute force FTW
                #define STEP 30.0
                const float alphaStep = (allowedAngles[0].t2 - allowedAngles[0].t1) / STEP;
                const float betaStep = (allowedAngles[1].t2 - allowedAngles[1].t1) / STEP;
                float nearest = 100000, alphaBest, betaBest;
                for (int i=0; i<STEP; i++) {
                    beta = allowedAngles[1].t1;
                    const Vector2 v = TRANSFORM(rotationE[0])->worldPosition + Vector2::Rotate(_rA, alpha);
                    for (int j=0; j<STEP; j++) {
                        float dist = Vector2::DistanceSquared(v + Vector2::Rotate(_rB, alpha + beta), target);
                        if (dist < nearest) {
                            nearest = dist;
                            alphaBest = alpha;
                            betaBest = beta;
                            TRANSFORM(dummy[0])->position = v + Vector2::Rotate(_rB, alpha + beta);
                        }
                        beta += betaStep;
                    }
                    alpha += alphaStep;
                }
                RENDERING(dummy[0])->hide = false;
                float aDiff = alphaBest - TRANSFORM(element[0])->rotation;
                TRANSFORM(element[0])->rotation += aDiff * dt * 5;
                float bDiff = betaBest - TRANSFORM(element[1])->rotation;
                TRANSFORM(element[1])->rotation += bDiff * dt * 5;
            }
        }
        float r = targetAbsRot - TRANSFORM(element[1])->rotation - TRANSFORM(element[0])->rotation;
        if (r < TRANSFORM(element[2])->rotation)
            TRANSFORM(element[2])->rotation = MathUtil::Max(allowedAngles[2].t1, r);
        else
            TRANSFORM(element[2])->rotation = MathUtil::Min(r, allowedAngles[2].t2);
        
    }
 #if 0
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            if (i == 2 || BUTTON(controls[2*i+j])->mouseOverIndex >= 0) {
                float pos = 0;
                if (i < 2)
                    pos = theTouchInputManager.getTouchLastPosition(BUTTON(controls[2*i+j])->mouseOverIndex).Y / (PlacementHelper::ScreenHeight * 0.5);
                else {
                    pos = theTouchInputManager.getPitch();
                    if (MathUtil::Abs(pos) < 0.05) pos = 0;
                }
                float a = TRANSFORM(element[i])->rotation + pos * rotSpeed[i] * dt;
                TRANSFORM(element[i])->rotation = MathUtil::Min(MathUtil::Max(allowedAngles[i].t1, a), allowedAngles[i].t2);
            }
        }
    }
#endif
#if 0
    for (int i=0; i<3; i++) {
        updateSlider(sliders[i]);
        {
            Vector2 r = TRANSFORM(element[i])->size * rotationOrigin[i];
            float angle = sliders[i].values.lerp(sliders[i].position);
            Vector2 origin = positions[i] * TRANSFORM(element[i])->size;
                
                TransformationSystem::setRotation(
                    TRANSFORM(element[i]), 
                    angle, 
                    r, origin);
                TRANSFORM(rotationE[i])->position = -r;
        }
    }
#endif
#if 0
    if (theTouchInputManager.isTouched(0)) {
        const Vector2 p = theTouchInputManager.getTouchLastPosition(0);
        if (!theTouchInputManager.wasTouched(0)) {
            for (int i=0; i<3; i++) {
                if (IntersectionUtil::pointRectangle(p, TRANSFORM(dummy[i])->worldPosition, TRANSFORM(dummy[i])->size)) {
                    which = i;
                    rotB = TRANSFORM(dummy[i])->rotation;
                    Vector2 ref = TRANSFORM(dummy[which])->worldPosition;
                    rotA = -MathUtil::AngleFromVector(p - ref);
                    break;
                }
            }
        }
        if (which >= 0) {
            Vector2 r = TRANSFORM(element[which])->size * rotationOrigin[which];
            Vector2 ref = TRANSFORM(dummy[which])->worldPosition;
            float angle = MathUtil::AngleFromVector(p - ref);
            Vector2 origin = positions[which] * TRANSFORM(element[which])->size;
            
            TransformationSystem::setRotation(
                TRANSFORM(element[which]), 
                rotB + rotA - angle, 
                r, origin);
            TRANSFORM(rotationE[which])->position = -r;
        }
    } else {
        which = -1;
    }
#endif

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

