/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PrototypeGame.h"

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"
#include "base/StateMachine.inl"

#include "systems/CameraSystem.h"
#include "systems/CornSystem.h"

#include "base/TimeUtil.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

PrototypeGame::PrototypeGame() : Game() { registerScenes(this, sceneStateMachine); }

void PrototypeGame::sacInitFromGameThread() {
    CornSystem::CreateInstance();
    Game::sacInitFromGameThread();
}

void PrototypeGame::preDestroy() { CornSystem::DestroyInstance(); }

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    sceneStateMachine.start(Scene::InGame);
}

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);

    // systems update
    theCornSystem.Update(dt);
}
