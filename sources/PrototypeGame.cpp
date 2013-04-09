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
#include "systems/ButtonSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/TaskAISystem.h"
#include "systems/MusicSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/CameraSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/GraphSystem.h"
#include "api/NetworkAPI.h"

#define ZOOM 1


PrototypeGame::PrototypeGame() : Game() {
}

bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        case ContextAPI::Localize:
        case ContextAPI::Communication:
            return true;
        default:
            return false;
    }
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theRenderingSystem.loadAtlas("font", true);
    // init font
    loadFont(renderThreadContext->assetAPI, "typo");
}

void PrototypeGame::init(const uint8_t*, int) {
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

    quickInit();

    pixelManager = new PixelManager("1.png", gameThreadContext->assetAPI);
}

void PrototypeGame::quickInit() {
}

void PrototypeGame::changeState(State::Enum newState) {
}

void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool) {

}

void PrototypeGame::tick(float dt) {
    if (theTouchInputManager.wasTouched(0) && !theTouchInputManager.isTouched(0))
    {
        glm::vec2 p = theTouchInputManager.getTouchLastPosition(0);
        pixelManager->clickedOn(p);
    }

    pixelManager->updatePixel();
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
