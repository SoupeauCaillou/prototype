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

#include "base/PlacementHelper.h"
#include "base/StateMachine.inl"
#include "base/EntityManager.h"

#include "systems/AnchorSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "base/TimeUtil.h"
#include "util/Random.h"

#include <algorithm>

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

PrototypeGame::PrototypeGame() : Game() {
    registerScenes(this, sceneStateMachine);
}

Cell::Cell() {
    for (int i=0; i<4; i++) {
        wallIndirect[i] = 0;
    }
    attr = cell_attibute::Out;
}

glm::vec2 PrototypeGame::firstCellPosition() {
    return glm::vec2(-0.5 * MAZE_SIZE, -0.5 * MAZE_SIZE)
                + glm::vec2(0.5f, 0.5f);
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    colors[0] = Color(0.9, 0.1, 0.05);
    Color::nameColor(colors[0], HASH("red", 0x08d91d68));
    colors[1] = Color(0.05, 0.1, 0.9);
    Color::nameColor(colors[1], HASH("blue", 0x975a0655));
    colors[2] = Color(0.5, 0.95, 0.05);
    Color::nameColor(colors[2], HASH("green", 0x615465c4));
    colors[3] = Color(0.9, 0.8, 0.1);
    Color::nameColor(colors[3], HASH("yellow", 0x74ae7e79));

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    sceneStateMachine.start(Scene::Menu);


    CAMERA(camera)->clearColor = Color(0.1, 0.1, 0.1);
    for (int i=0; i<4; i++) {
        guy[i] = theEntityManager.CreateEntityFromTemplate("guy");
        RENDERING(guy[i])->color = colors[i];
    }
}


#include "util/Draw.h"
void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);
}






