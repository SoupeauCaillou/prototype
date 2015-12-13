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

#include "PlayerSystem.h"
#include "base/TimeUtil.h"
#include "util/Random.h"

#include "api/KeyboardInputHandlerAPI.h"
#include <SDL2/SDL.h>
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

    PlayerSystem::CreateInstance();
    thePlayerSystem.game = this;
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

    round = 0;

    sceneStateMachine.start(Scene::Menu);

    glm::vec2 hCamSize = TRANSFORM(camera)->size * .5f;
    CAMERA(camera)->clearColor = Color(0.1, 0.1, 0.1);
    for (int i=0; i<4; i++) {
        guy[i] = theEntityManager.CreateEntityFromTemplate("guy");
        RENDERING(guy[i])->color = colors[i];

        ui[i].root = theEntityManager.CreateEntityFromTemplate("score_root");
        ui[i].score = theEntityManager.CreateEntityFromTemplate("score_text");
        ui[i].bet = theEntityManager.CreateEntityFromTemplate("score_bet");
        ANCHOR(ui[i].score)->parent = ui[i].root;
        ANCHOR(ui[i].bet)->parent = ui[i].root;
        TRANSFORM(ui[i].root)->size = hCamSize - glm::vec2(MAZE_SIZE * 0.5f, 0);
        RENDERING(ui[i].root)->color = colors[i];

        TRANSFORM(ui[i].score)->position = glm::vec2(
            TRANSFORM(camera)->size.x * 0.5,
            0);

        glm::vec2 absPosition = glm::vec2(
            (i % 2) ? hCamSize.x : -hCamSize.x,
            (i < 2) ? hCamSize.y : -hCamSize.y);
        Cardinal::Enum cardinals[] = {
            Cardinal::NW, Cardinal::NE, Cardinal::SW, Cardinal::SE
        };
        TRANSFORM(ui[i].root)->position = AnchorSystem::adjustPositionWithCardinal(
            absPosition, TRANSFORM(ui[i].root)->size,
            cardinals[i]);

        ANCHOR(ui[i].score)->position = glm::vec2(
            TRANSFORM(ui[i].root)->size.x * -0.5 + 0.1,
            TRANSFORM(ui[i].root)->size.y * 0.5 - 1);
    }
}

void PrototypeGame::tick(float dt) {
    int key2Dir[] = { SDLK_UP, SDLK_RIGHT, SDLK_DOWN, SDLK_LEFT };
    for (int i=0; i<4; i++) {
        if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(key2Dir[i])) {
            PLAYER(guy[0])->input.directions[i] = InputState::Pressed;
        } else if (gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(key2Dir[i])) {
            PLAYER(guy[0])->input.directions[i] = InputState::Released;
        } else {
            PLAYER(guy[0])->input.directions[i] = InputState::None;
        }
    }

    sceneStateMachine.update(dt);
}






