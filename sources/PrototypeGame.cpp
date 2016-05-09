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

#define SAC_TWEAK_IMPLEMENTATION
#include "PrototypeGame.h"

#include "base/PlacementHelper.h"
#include "base/StateMachine.inl"
#include "base/EntityManager.h"

#include "systems/CameraSystem.h"

#include "base/TimeUtil.h"
#include "api/KeyboardInputHandlerAPI.h"
#include "api/JoystickAPI.h"
#include <SDL2/SDL.h>

#include "PlayerSystem.h"
#include "EquipmentSystem.h"
#include "SwordSystem.h"
#include "HealthSystem.h"
#include "GunSystem.h"
#include "IASystem.h"
#include "systems/AnchorSystem.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

PrototypeGame::PrototypeGame() : Game() {
    PlayerSystem::CreateInstance();
    thePlayerSystem.game = this;
    EquipmentSystem::CreateInstance();
    theEquipmentSystem.game = this;
    SwordSystem::CreateInstance();
    theSwordSystem.game = this;
    HealthSystem::CreateInstance();
    theHealthSystem.game = this;
    GunSystem::CreateInstance();
    theGunSystem.game = this;
    IASystem::CreateInstance();
    theIASystem.game = this;


    registerScenes(this, sceneStateMachine);
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    sceneStateMachine.start(Scene::Menu);

    battleground = theEntityManager.CreateEntityFromTemplate("battleground");
    guy[0] = theEntityManager.CreateEntityFromTemplate("guy");
    EQUIPMENT(guy[0])->hand.right = theEntityManager.CreateEntityFromTemplate("sword");
    // EQUIPMENT(guy[0])->hand.left = theEntityManager.CreateEntityFromTemplate("sword");
    // RENDERING(EQUIPMENT(guy[0])->hand.left)->color.r = 0;
}

static InputState::Enum keyToState(KeyboardInputHandlerAPI* kb, int key) {
    if (kb->isKeyPressed(key)) {
        return InputState::Pressed;
    } else if (kb->isKeyReleased(key)) {
        return InputState::Released;
    } else {
        return InputState::None;
    }
 }

 static glm::vec2 keysToDirection(KeyboardInputHandlerAPI* kb, int* keys) {
    glm::vec2 dir(0.0f);
    if (keyToState(kb, keys[0]) == InputState::Pressed) {
        dir.y = 1.0f;
    } else if (keyToState(kb, keys[1]) == InputState::Pressed) {
        dir.y = -1.0f;
    }
    if (keyToState(kb, keys[2]) == InputState::Pressed) {
        dir.x = 1.0f;
    } else if (keyToState(kb, keys[3]) == InputState::Pressed) {
        dir.x = -1.0f;
    }
    return dir;
 }

 void PrototypeGame::tick(float dt) {
    /* keyboard control */
    int key2DirPrimary[] = { SDLK_UP, SDLK_DOWN, SDLK_RIGHT, SDLK_LEFT };
    int key2DirSecondary[] = { SDLK_z, SDLK_s, SDLK_d, SDLK_q };
    auto* kb = gameThreadContext->keyboardInputHandlerAPI;

    for (int j=0; j<4;j++) {
        PLAYER(guy[j])->input.directions.primary = keysToDirection(kb, key2DirPrimary);
        PLAYER(guy[j])->input.directions.secondary = keysToDirection(kb, key2DirSecondary);

        PLAYER(guy[j])->input.actions[0] = keyToState(kb, SDLK_SPACE);
        PLAYER(guy[j])->input.actions[1] = keyToState(kb, SDLK_LSHIFT);
        break;
    }

    /* joystick control */
    if (gameThreadContext->joystickAPI->availableJoystick()) {
        PLAYER(guy[0])->input.directions.primary = gameThreadContext->joystickAPI->getPadDirection(0, 0);
        PLAYER(guy[0])->input.directions.secondary = gameThreadContext->joystickAPI->getPadDirection(0, 1);
        if (gameThreadContext->joystickAPI->hasClicked(0, 0)) {
            PLAYER(guy[0])->input.actions[0] = InputState::Released;
        } else if (gameThreadContext->joystickAPI->isDown(0, 0)) {
            PLAYER(guy[0])->input.actions[0] = InputState::Pressed;
        } else {
            PLAYER(guy[0])->input.actions[0] = InputState::None;
        }

        if (gameThreadContext->joystickAPI->hasClicked(0, 1)) {
            PLAYER(guy[0])->input.actions[1] = InputState::Released;
        } else if (gameThreadContext->joystickAPI->isDown(0, 1)) {
            PLAYER(guy[0])->input.actions[1] = InputState::Pressed;
        } else {
            PLAYER(guy[0])->input.actions[1] = InputState::None;
        }

    }

    sceneStateMachine.update(dt);
}
