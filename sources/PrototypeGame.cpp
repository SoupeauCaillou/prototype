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

#include "systems/CameraSystem.h"
#include "systems/SheepSystem.h"

#include "util/PrototypeDebugConsole.h"
    
#define ZOOM 1

#include <ostream>
#include <fstream>
#if SAC_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

PrototypeGame::PrototypeGame(int, char**) : Game() {
    SheepSystem::CreateInstance();

    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::GameStart, Scene::CreateGameStartSceneHandler(this), "Scene::GameStart");
    sceneStateMachine.registerState(Scene::InGame, Scene::CreateInGameSceneHandler(this), "Scene::InGame");
    sceneStateMachine.registerState(Scene::GameEnd, Scene::CreateGameEndSceneHandler(this), "Scene::GameEnd");
    sceneStateMachine.registerState(Scene::Editor, Scene::CreateEditorSceneHandler(this), "Scene::Editor");
    LOGF_IF(sceneStateMachine.getStateCount() != (int)Scene::Count,
        "Missing " << (int)Scene::Count - sceneStateMachine.getStateCount() << " state handler(s)");
}

PrototypeGame::~PrototypeGame() {
    LOGW("Delete game instance " << this << " " << &theEntityManager);
    SheepSystem::DestroyInstance();
    
    theEntityManager.deleteAllEntities();
}


bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        case ContextAPI::Localize:
        case ContextAPI::Sound:
            return true;
#if SAC_DESKTOP
        case ContextAPI::KeyboardInputHandler:
            return true;
#endif
        default:
            return false;
    }
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");

    Game::sacInit(windowW, windowH);

    PlacementHelper::GimpSize = glm::vec2(1280, 800);

    LOGI("SAC engine initialisation done.");
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    // default camera
    camera = theEntityManager.CreateEntityFromTemplate("camera");
    faderHelper.init(camera);

    sceneStateMachine.setup();
    #if SAC_DEBUG
        sceneStateMachine.start(Scene::Menu);
    #else
        sceneStateMachine.start(Scene::Logo);
    #endif


    #if SAC_INGAME_EDITORS && SAC_DEBUG
        PrototypeDebugConsole::init(this);
    #endif

    levelLoader.init(gameThreadContext->assetAPI);
    saveManager.init(this);
    saveManager.load();

    currentLevel = 0;
    auto list = gameThreadContext->assetAPI->listAssetContent(".ini", "maps");
    //i want a vector!
    std::copy(list.begin(), list.end(), back_inserter(levels));

    LOGF_IF(levels.size() == 0, "Could not found any level -> force crash");
    LOGI("PrototypeGame initialisation done.");
}

void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool) {

}

void PrototypeGame::tick(float dt) {
    sceneStateMachine.update(dt);

}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}

void PrototypeGame::saveLevelProgression(bool timeDone, float time, bool sheepDone, int deadSheep) {
    saveManager.setValue("level_finished_" + levels[currentLevel], "1");
    saveManager.setValue("level_time_done_" + levels[currentLevel], std::to_string(timeDone));
    saveManager.setValue("level_time" + levels[currentLevel], std::to_string(time));
    saveManager.setValue("level_sheep_done_" + levels[currentLevel], std::to_string(sheepDone));
    saveManager.setValue("level_sheep_dead_" + levels[currentLevel], std::to_string(deadSheep));
}
