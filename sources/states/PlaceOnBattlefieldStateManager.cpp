/*
    This file is part of Brikwars.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    RecursiveRunner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    RecursiveRunner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "StateManager.h"
#include "../PrototypeGame.h"
#include "base/EntityManager.h"
#include "base/Vector2.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/FighterSystem.h"
#include "systems/PickableSystem.h"
#include "systems/CameraDepSystem.h"
#include "systems/CameraTargetSystem.h"
#include <vector>
#include <sstream>

struct PlaceOnBattlefieldStateManager::PlaceOnBattlefieldStateManagerDatas {
    Entity activeFighter;
    Entity selection;
    float colorEvolution;
};

PlaceOnBattlefieldStateManager::PlaceOnBattlefieldStateManager(PrototypeGame* game) : StateManager(State::PlaceOnBattlefield, game) {
    datas = new PlaceOnBattlefieldStateManagerDatas;
}

PlaceOnBattlefieldStateManager::~PlaceOnBattlefieldStateManager() {
    delete datas;
}

void PlaceOnBattlefieldStateManager::setup() {
    Entity selection = datas->selection = theEntityManager.CreateEntity();
    ADD_COMPONENT(selection, Transformation);
    TRANSFORM(selection)->z = -0.1;
    ADD_COMPONENT(selection, Rendering);
    RENDERING(selection)->color = Color(1, 0, 0);
}


///----------------------------------------------------------------------------//
///--------------------- ENTER SECTION ----------------------------------------//
///----------------------------------------------------------------------------//
void PlaceOnBattlefieldStateManager::willEnter(State::Enum from) {
    std::vector<Entity> fighters = theFighterSystem.RetrieveAllEntityWithComponent();
    std::vector<Entity> players = thePlayerSystem.RetrieveAllEntityWithComponent();
    // create UI
    const float sizeRatioY = 1.0 / 12.0;
    const float sizeRatioX = sizeRatioY * theRenderingSystem.screenH / theRenderingSystem.screenW;
    
    for (unsigned j=0; j<2; j++) {
        int count = 0;
        for (unsigned i=0; i<fighters.size(); i++) {
            if (FIGHTER(fighters[i])->player == players[j]) {
                Entity icon = theEntityManager.CreateEntity();
                ADD_COMPONENT(icon, Transformation);
                TRANSFORM(icon)->z = 0.9;
                ADD_COMPONENT(icon, CameraDep);
                CAMERA_DEP(icon)->screenScalePosition = Vector2(
                    -0.5 + sizeRatioX * 0.5 + j * (1-sizeRatioX),
                    0.5 - sizeRatioY * 0.5 - count * sizeRatioY);
                CAMERA_DEP(icon)->screenScaleSize = Vector2(sizeRatioX, sizeRatioY);
                CAMERA_DEP(icon)->cameraIndex = 0;
                ADD_COMPONENT(icon, Rendering);
                RENDERING(icon)->texture = theRenderingSystem.loadTextureFile("head");
                RENDERING(icon)->hide = false;
                ADD_COMPONENT(icon, Button);
                BUTTON(icon)->enabled = true;
                game->inGameUI.fightersIcons.push_back(std::make_pair(icon, fighters[i]));
                count++;
            }
        }
    }
    datas->activeFighter = 0;
    datas->colorEvolution = -1;
}

bool PlaceOnBattlefieldStateManager::transitionCanEnter(State::Enum) {
    return true;
}


void PlaceOnBattlefieldStateManager::enter(State::Enum) {

}

///----------------------------------------------------------------------------//
///--------------------- UPDATE SECTION ---------------------------------------//
///----------------------------------------------------------------------------//
void PlaceOnBattlefieldStateManager::backgroundUpdate(float) {
}

State::Enum PlaceOnBattlefieldStateManager::update(float dt) {
    for (unsigned i=0; i<game->inGameUI.fightersIcons.size(); i++) {
        if (BUTTON(game->inGameUI.fightersIcons[i].first)->clicked) {
            // disable every other
            std::vector<Entity> camTargets = theCameraTargetSystem.RetrieveAllEntityWithComponent();
            for (unsigned j=0; j<camTargets.size(); j++)
                CAM_TARGET(camTargets[j])->enabled = false;
            // move camera on fighter
            Entity fighter = game->inGameUI.fightersIcons[i].second;
            if (fighter == datas->activeFighter) {
                datas->activeFighter = 0;
                RENDERING(datas->selection)->hide = true;
                CAM_TARGET(fighter)->enabled = false;
            } else {
                CAM_TARGET(fighter)->enabled = true;
                CAM_TARGET(fighter)->limits.min = Vector2(-30, -20) + theRenderingSystem.cameras[0].worldSize * 0.5;
                CAM_TARGET(fighter)->limits.max = Vector2(30, 20) - theRenderingSystem.cameras[0].worldSize * 0.5;
                datas->activeFighter = fighter;
                TRANSFORM(datas->selection)->parent = fighter;
                TRANSFORM(datas->selection)->size = TRANSFORM(FIGHTER(fighter)->torso)->size * Vector2(1.8, 0.25);
                TRANSFORM(datas->selection)->position = TRANSFORM(FIGHTER(fighter)->leg[0])->size * Vector2(0, -1.55);
                RENDERING(datas->selection)->hide = false;
            }
            break;
        }
    }
    RENDERING(datas->selection)->color.a += datas->colorEvolution * dt;
    if (datas->colorEvolution < 0 && RENDERING(datas->selection)->color.a <= 0.1) {
        RENDERING(datas->selection)->color.a = 0.1;
        datas->colorEvolution = 1;
    } else if (datas->colorEvolution > 0 && RENDERING(datas->selection)->color.a >= 1) {
        RENDERING(datas->selection)->color.a = 1;
        datas->colorEvolution = -1;
    }

    if (datas->activeFighter) {
        if (theTouchInputManager.wasTouched(0) && !theTouchInputManager.isTouched(0)) {
            if (MathUtil::Abs(theTouchInputManager.getTouchLastScreenPosition(0).X) > theRenderingSystem.screenW * 0.4) {
                std::cout << "ignore click" << std::endl;
            } else {
                TRANSFORM(datas->activeFighter)->position = theTouchInputManager.getTouchLastPosition(0);
                CAM_TARGET(datas->activeFighter)->enabled = false;
            }
        }
    }

    return State::PlaceOnBattlefield; //BattleColorPick;
}


///----------------------------------------------------------------------------//
///--------------------- EXIT SECTION -----------------------------------------//
///----------------------------------------------------------------------------//
void PlaceOnBattlefieldStateManager::willExit(State::Enum) {
}

bool PlaceOnBattlefieldStateManager::transitionCanExit(State::Enum) {
    return true;
}

void PlaceOnBattlefieldStateManager::exit(State::Enum) {
    
}
