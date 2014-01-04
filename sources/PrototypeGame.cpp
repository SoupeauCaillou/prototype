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

#include "systems/CameraSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"
#include "WeaponSystem.h"
#include "SoldierSystem.h"
#include "BulletSystem.h"
#include "MessageSystem.h"
#include "PlayerSystem.h"
#include "TeamSystem.h"
#include "FlagSystem.h"
#include "SelectionSystem.h"
#include "util/Random.h"

#include <ostream>
#include <fstream>
#if SAC_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif

#include "base/StateMachine.inl"
#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

PrototypeGame::PrototypeGame(int argc, char** argv) : Game(), serverIp(""), nickName("johndoe"){
#if SAC_LINUX && SAC_DESKTOP
    char* nick = getlogin();
    if (nick)
        nickName = nick;
#endif

    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-server") == 0) {
            LOGF_IF(i + 1 >= argc, "Incorrect #args");
            serverIp = argv[++i];
        }
        else if (strcmp(argv[i], "-nick") == 0) {
            LOGF_IF(i + 1 >= argc, "Incorrect #args");
            nickName = argv[++i];
        }
    }
    myPlayer = 0;

    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::GameStart, Scene::CreateGameStartSceneHandler(this), "Scene::GameStart");
    sceneStateMachine.registerState(Scene::Active, Scene::CreateActiveSceneHandler(this), "Scene::Active");
    sceneStateMachine.registerState(Scene::Paused, Scene::CreatePausedSceneHandler(this), "Scene::Paused");
    sceneStateMachine.registerState(Scene::GameEnd, Scene::CreateGameEndSceneHandler(this), "Scene::GameEnd");
    LOGF_IF(sceneStateMachine.getStateCount() != (int)Scene::Count,
        "Missing " << (int)Scene::Count - sceneStateMachine.getStateCount() << " state handler(s)");
}

bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
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

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");

    WeaponSystem::CreateInstance();
    // only updated by game's host
    // orderedSystemsToUpdate.push_back(WeaponSystem::GetInstancePointer());
    SoldierSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(SoldierSystem::GetInstancePointer());
    BulletSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(BulletSystem::GetInstancePointer());
    MessageSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(MessageSystem::GetInstancePointer());
    PlayerSystem::CreateInstance();
    orderedSystemsToUpdate.push_back(PlayerSystem::GetInstancePointer());
    TeamSystem::CreateInstance(); // only on game's host
    FlagSystem::CreateInstance(); // only on game's host
    SelectionSystem::CreateInstance();
    theSelectionSystem.kbApi = gameThreadContext->keyboardInputHandlerAPI;

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

    LOGI("PrototypeGame initialisation done.");

    if (!serverIp.empty()) {
        // init network connection
        auto* api = gameThreadContext->networkAPI;
        // connect to lobby
        api->connectToLobby(nickName, serverIp.c_str());
    } else {
        LOGI("Solo game");
    }

    theEntityManager.CreateEntityFromTemplate("arena");

    theCollisionSystem.worldSize = glm::vec2(40, 40);//PlacementHelper::ScreenSize.x, PlacementHelper::ScreenSize.x);

    glm::vec2 maxSize(
        theCollisionSystem.worldSize.x * (TRANSFORM(camera)->size.x / TRANSFORM(camera)->size.y),
        theCollisionSystem.worldSize.x);
    cameraMoveManager.init(camera, maxSize * 0.5f, 5, 
        glm::vec4(maxSize * -0.5f, maxSize * 0.5f));
    cameraMoveManager.setZoom(2);

    FileBuffer fb = gameThreadContext->assetAPI->loadAsset("config.ini");
    config.load(fb, "config.ini");
    delete[] fb.data;

    if (serverIp.empty()) {
        if (!config.get("", "server", &serverIp, 1)) {
            serverIp = "127.0.0.1";
        }
    }

    timer = theEntityManager.CreateEntityFromTemplate("timer");

#if SAC_DEBUG
    sceneStateMachine.start(Scene::Menu);
#else
    sceneStateMachine.start(Scene::Logo);
#endif
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

void PrototypeGame::oneTimeGameSetup(const std::map<std::string, NetworkStatus::Enum>& playersInGame) {
    if (isGameHost) {
        Color colors[] = {
            Color(0.8, 0.2, 0.2),
            Color(0.2, 0.2, 0.8),
            Color(0.2, 0.8, 0.2),
            Color(0.6, 0.3, 0.6)
        };
        // create teams
        int index = 0;
        if (playersInGame.empty()) {
            Entity t = theEntityManager.CreateEntityFromTemplate("team");
            TEAM(t)->index = 0;
            TEAM(t)->color = colors[0];
            TEAM(t)->name = nickName;

            Entity spawn = theEntityManager.CreateEntityFromTemplate("spawn");
            RENDERING(spawn)->color = TEAM(t)->color * 0.5;
            TEAM(t)->spawn = spawn;
        } else {
            for (auto it=playersInGame.begin(); it!=playersInGame.end(); ++it, index++) {
                Entity t = theEntityManager.CreateEntityFromTemplate("team");
                TEAM(t)->index = index;
                TEAM(t)->color = colors[index];
                TEAM(t)->name = it->first;

                Entity spawn = theEntityManager.CreateEntityFromTemplate("spawn");
                RENDERING(spawn)->color = TEAM(t)->color * 0.5;
                TRANSFORM(spawn)->position = glm::rotate(TRANSFORM(spawn)->position, (TEAM(t)->index * 2.0f * glm::pi<float>()) / theTeamSystem.entityCount());
                TEAM(t)->spawn = spawn;
            }
        }
        theEntityManager.CreateEntityFromTemplate("flag");
    }
}

bool PrototypeGame::eachTimeGameSetup() {
    Entity team = 0;
    theTeamSystem.forEachECDo([this, &team] (Entity e, TeamComponent* tc) -> void {
        if (tc->name == nickName) {
            team = e;
        }
    });

    if (!team || !TEAM(team)->spawn)
        return false;

    if (isGameHost) {
        Entity b = 0;
        while ((b = theEntityManager.getEntityByName("block"))) {
            theEntityManager.DeleteEntity(b);
        }
        int bCount = 10;
        config.get("Arena", "block_count", &bCount);
        for (int i=0; i<bCount; i++) {
            theEntityManager.CreateEntityFromTemplate("block");
        }
        // reset flag position
        Entity flag = theEntityManager.getEntityByName("flag");
        ANCHOR(flag)->parent = 0;
        TRANSFORM(flag)->position = glm::vec2(0.0f);
    }


    // destroy soldiers
    for (unsigned i=0; i<players.size(); i++) {
        theEntityManager.DeleteEntity(SOLDIER(players[i])->weapon);
        theEntityManager.DeleteEntity(SELECTION(players[i])->icon);
        theEntityManager.DeleteEntity(players[i]);
    }
    players.clear();

    auto* spawn = TRANSFORM(TEAM(team)->spawn);
    int soldierCount = 3;
    config.get("", "soldier_count", &soldierCount);
    const std::string soldier[] = {"gunman", "shotgunman", "machinegunman"};
    for (int i=0; i<soldierCount; i++) {
        Entity p = theEntityManager.CreateEntityFromTemplate(soldier[i % 3]);
        RENDERING(p)->color = TEAM(team)->color;
        SOLDIER(p)->team = team;
        SELECTION(p)->keyScanCode = 10 + i;
        LOGF_IF(i > JoystickButton::TOTAL, "We do support only " << JoystickButton::TOTAL << " soldiers");
        SELECTION(p)->joystickBtn = (JoystickButton::Enum)i;
        ANCHOR(SOLDIER(p)->weapon)->parent = p;
        players.push_back(p);

        // position randomly in the spawn arena
        TRANSFORM(p)->position = spawn->position +
            glm::rotate(spawn->size * glm::vec2(Random::Float(-0.45, 0.45), Random::Float(-0.45, 0.45)), spawn->rotation);
    }
    return true;
}
