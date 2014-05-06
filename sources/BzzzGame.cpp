/*
    This file is part of Bzzz.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Bzzz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Bzzz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bzzz.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "BzzzGame.h"

#include "base/PlacementHelper.h"
#include "base/StateMachine.inl"

#include "systems/CameraSystem.h"

#include "util/Random.h"
#include "systems/TransformationSystem.h"
#include <glm/gtx/vector_angle.hpp>


#include <ostream>
#include <fstream>
#if SAC_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

#include "util/Random.h"

BzzzGame::BzzzGame(int argc, char** argv) : Game() {
    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::GameStart, Scene::CreateGameStartSceneHandler(this), "Scene::GameStart");
    sceneStateMachine.registerState(Scene::InGame, Scene::CreateInGameSceneHandler(this), "Scene::InGame");
    sceneStateMachine.registerState(Scene::GameEnd, Scene::CreateGameEndSceneHandler(this), "Scene::GameEnd");
    LOGF_IF(sceneStateMachine.getStateCount() != (int)Scene::Count,
        "Missing " << (int)Scene::Count - sceneStateMachine.getStateCount() << " state handler(s)");
}

bool BzzzGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        case ContextAPI::Localize:
        case ContextAPI::Communication:
        case ContextAPI::Sound:
#if SAC_NETWORK
        case ContextAPI::Network:
#endif
        case ContextAPI::KeyboardInputHandler:
        case ContextAPI::WWW:
            return true;
        default:
            return false;
    }
}

void BzzzGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");

    Game::sacInit(windowW, windowH);

    PlacementHelper::GimpSize = glm::vec2(1280, 800);

    LOGI("SAC engine initialisation done.");
}

void BzzzGame::init(const uint8_t*, int) {
    LOGI("BzzzGame initialisation begins...");

    // load config
    {
        FileBuffer fb = gameThreadContext->assetAPI->loadAsset("params.ini");

        parameters.load(fb, "params.ini");

        char* p = (char*)alloca(strlen("player_N") + 1);
        for (int i=0; i<5; i++) {
            sprintf(p, "player_%d", i);
            parameters.get("Colors", p, playerColors[i].rgba, 4);
        }

        delete[] fb.data;
    }

    int defaultActivePlayer = Random::Int(0, 3);
    for (int i=0; i<4; i++) {
        playerActive[i] = (defaultActivePlayer == i) ? 0 : -1;
    }

    {
        char* tmp = (char*) alloca(strlen("menu/player_button_N") + 1);
        for (int i=0; i<4; i++) {
            sprintf(tmp, "menu/player_button_%d", i + 1);
            playerButtons[i] = theEntityManager.CreateEntityFromTemplate(tmp);
            RENDERING(playerButtons[i])->color = playerColors[(defaultActivePlayer == i) ? (i+1) : 0];

            score[i] = 0;
        }
    }

    // default camera
    camera = theEntityManager.CreateEntityFromTemplate("camera");
    faderHelper.init(camera);

    sceneStateMachine.setup();
#if SAC_DEBUG
    sceneStateMachine.start(Scene::Logo);
#else
    sceneStateMachine.start(Scene::Logo);
#endif
}

void BzzzGame::backPressed() {
}

void BzzzGame::togglePause(bool) {

}

void BzzzGame::tick(float dt) {
    sceneStateMachine.update(dt);
}

bool BzzzGame::willConsumeBackEvent() {
    return false;
}

void BzzzGame::beesPopping(Entity fromBtn) {
    return;
    const glm::vec2 center = TRANSFORM(fromBtn)->position;
    const glm::vec2 size = TRANSFORM(fromBtn)->size;
    Color def = RENDERING(fromBtn)->color;

    for (int i = 0; i < 25; ++i) {
        Entity e = theEntityManager.CreateEntityFromTemplate("menu/popping_bee");
        TransformationComponent * tc = TRANSFORM(e);
        tc->position = center - size / 2.f;
        tc->position.x += Random::Float(0.f, size.x);
        tc->position.y += Random::Float(0.f, size.y);

        Color gradient = Color::random();
        RENDERING(e)->color = def + gradient.reducePrecision(.3f);
        tc->rotation = glm::orientedAngle(glm::vec2(1, 0), glm::normalize(-center));
    }
}
