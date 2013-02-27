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
#include "Fond_ecranGame.h"
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

Fond_ecranGame::Fond_ecranGame(AssetAPI* ast) : Game() {
    asset = ast;
}

void Fond_ecranGame::sacInit(int windowW, int windowH) {
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theRenderingSystem.loadAtlas("alphabet", true);
    //-theRenderingSystem.loadAtlas("logo", false);

    // init font
    loadFont(asset, "typo");
}

void Fond_ecranGame::init(const uint8_t*, int) {
    quickInit();

    std::stringstream s;
    s << MathUtil::RandomIntInRange(1, 3) << ".png";
    thePixelManager = new PixelManager(s.str(), asset);
}

void Fond_ecranGame::quickInit() {
}

void Fond_ecranGame::backPressed() {
}

void Fond_ecranGame::togglePause(bool) {
}

void Fond_ecranGame::tick(float dt) {
    Vector2 p = Vector2(-999, -999);

    if (theTouchInputManager.isTouched(0))
    {
        p = theTouchInputManager.getTouchLastPosition(0);
    }

    if (theTouchInputManager.wasTouched(0) && !theTouchInputManager.isTouched(0) && p != Vector2(-999, -999))
    {
        thePixelManager->clickedOn(p);
    }

    thePixelManager->updatePixel();
    thePixelManager->updatePixel();
    thePixelManager->updatePixel();
    thePixelManager->updatePixel();
    thePixelManager->updatePixel();
    thePixelManager->updatePixel();
}

bool Fond_ecranGame::willConsumeBackEvent() {
    return false;
}
