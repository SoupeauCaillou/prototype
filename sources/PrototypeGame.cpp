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

#include "api/NetworkAPI.h"

#include "base/PlacementHelper.h"
#include "util/IntersectionUtil.h"

#include "systems/CameraSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ActionSystem.h"
#include "systems/MorpionGridSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/TicTacToeSystem.h"

#define ZOOM 1

#include <ostream>
#include <fstream>
#if SAC_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

PrototypeGame::PrototypeGame(int argc, char** argv) : Game() {
    networkMode = false;
    lobbyAddress = "127.0.0.1";
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') {
            LOGI("Your networkNickname is '" << argv[i] << "'");
            networkNickname = argv[i];
            networkMode = true;
        } else {
            if (strcmp(argv[i], "-server") == 0) {
                LOGF_IF(argc < (i+1), "Missing param to -server arg");
                lobbyAddress = argv[i + 1];
                LOGI("Using lobby at: '" << lobbyAddress << "'");
                i++;
            }
        }
    }

    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::GameStart, Scene::CreateGameStartSceneHandler(this), "Scene::GameStart");
    sceneStateMachine.registerState(Scene::TurnStart, Scene::CreateTurnStartSceneHandler(this), "Scene::TurnStart");
    sceneStateMachine.registerState(Scene::TurnEnd, Scene::CreateTurnEndSceneHandler(this), "Scene::TurnEnd");
    sceneStateMachine.registerState(Scene::GameEnd, Scene::CreateGameEndSceneHandler(this), "Scene::GameEnd");
    LOGF_IF(sceneStateMachine.getStateCount() != (int)Scene::Count,
        "Missing " << (int)Scene::Count - sceneStateMachine.getStateCount() << " state handler(s)");
}

bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
            return true;
        case ContextAPI::Network:
            return networkMode;
        default:
            return false;
    }
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");

    Game::sacInit(windowW, windowH);

    PlacementHelper::GimpSize = glm::vec2(1280, 800);

    theRenderingSystem.createFramebuffer("ui_fb", windowW, windowH);
    LOGI("SAC engine initialisation done.");
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    ActionSystem::CreateInstance();
    MorpionGridSystem::CreateInstance();
    PlayerSystem::CreateInstance();
    TicTacToeSystem::CreateInstance();

#if SAC_DEBUG
    sceneStateMachine.setup(Scene::Menu);
#else
    sceneStateMachine.setup(Scene::Logo);
#endif

    // default camera
    camera = theEntityManager.CreateEntity("camera",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("camera"));

    quickInit();
    LOGI("PrototypeGame initialisation done.");
}

void PrototypeGame::quickInit() {

}

void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool) {

}

void PrototypeGame::tick(float dt) {
    bool gameMaster = (!networkMode || gameThreadContext->networkAPI->amIGameMaster());
    if (dt > 0) {
        sceneStateMachine.update(dt);
        if (gameMaster) {
            theActionSystem.Update(dt);
            theMorpionGridSystem.Update(dt);
        }
    }
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
