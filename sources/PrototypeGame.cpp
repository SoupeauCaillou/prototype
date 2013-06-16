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
#include "base/PlacementHelper.h"

#include "util/IntersectionUtil.h"

#include "systems/TransformationSystem.h"
#include "systems/SoldierSystem.h"
#include "systems/ActionSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/TacticalAISystem.h"
#include "systems/VisionSystem.h"

#if SAC_INGAME_EDITORS
#include "util/PrototypeDebugConsole.h"
#endif

#define ZOOM 1

#include <ostream>
#include <fstream>
    #if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

PrototypeGame::PrototypeGame(int, char**) : Game(), grid(39, 27, 1.1) {
    nickname = "anonymous";
    serverIp = "127.0.0.1";

    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
#if SAC_NETWORK
    sceneStateMachine.registerState(Scene::Connecting, Scene::CreateConnectingSceneHandler(this), "Scene::Connecting");
#endif
    sceneStateMachine.registerState(Scene::SocialCenter, Scene::CreateSocialCenterSceneHandler(this), "Scene::SocialCenter");
    sceneStateMachine.registerState(Scene::SelectCharacter, Scene::CreateSelectCharacterSceneHandler(this), "Scene::SelectCharacter");
    sceneStateMachine.registerState(Scene::SelectAction, Scene::CreateSelectActionSceneHandler(this), "Scene::SelectAction");
    sceneStateMachine.registerState(Scene::ExecuteAction, Scene::CreateExecuteActionSceneHandler(this), "Scene::ExecuteAction");
    sceneStateMachine.registerState(Scene::BeginTurn, Scene::CreateBeginTurnSceneHandler(this), "Scene::BeginTurn");
    sceneStateMachine.registerState(Scene::EndTurn, Scene::CreateEndTurnSceneHandler(this), "Scene::EndTurn");
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
            return true;
        default:
            return false;
    }
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");
    SoldierSystem::CreateInstance();
    VisionSystem::CreateInstance();
    ActionSystem::CreateInstance();
    PlayerSystem::CreateInstance();
    TacticalAISystem::CreateInstance();

    Game::sacInit(windowW, windowH);

    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

    theActionSystem.game = this;
    theVisionSystem.game = this;

    LOGI("SAC engine initialisation done.");
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");
#if SAC_DEBUG
    sceneStateMachine.setup(Scene::Menu);
#else
    sceneStateMachine.setup(Scene::Logo);
#endif
    sceneStateMachine.reEnterCurrentState();

    // default camera
    camera = theEntityManager.CreateEntity("camera",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("camera"));

#if SAC_INGAME_EDITORS
    PrototypeDebugConsole::init(this);
#endif

    grid.doForEachCell([this] (const GridPos& p) -> void {
        Entity e = theEntityManager.CreateEntity("gridcell",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
        TRANSFORM(e)->position = this->grid.gridPosToPosition(p);
        grid.addEntityAt(e, p);
    });

    quickInit();


    humanPlayer = theEntityManager.CreateEntity("human_player",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("human_player"));
    aiPlayer = theEntityManager.CreateEntity("ai_player",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("ai_player"));

    std::stringstream a;
    for (int i=1; i<27; ++i) {
        a.str("");
        a << "wall_" << i;
        Entity wall = theEntityManager.CreateEntity(a.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
        walls.push_back(wall);
    }

    for (int i=1; i<10; ++i) {
        a.str("");
        a << "yellowSoldier_" << i;
        Entity s = theEntityManager.CreateEntity(a.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
        yEnnemies.push_back(s);
        SOLDIER(s)->player = aiPlayer;
    }

    for (int i=1; i<3; ++i) {
        a.str("");
        a << "blueSoldier_" << i;
        Entity s = theEntityManager.CreateEntity(a.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
        bEnnemies.push_back(s);
        SOLDIER(s)->player = aiPlayer;
    }


    for (int i=1; i<3; ++i) {
        a.str("");
        a << "objective_" << i;
        Entity s = theEntityManager.CreateEntity(a.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
        objs.push_back(s);
    }

    players.push_back(theEntityManager.CreateEntity("playerb",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playerb")));
    players.push_back(theEntityManager.CreateEntity("playerg",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playerg")));
    players.push_back(theEntityManager.CreateEntity("playerr",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playerr")));
    players.push_back(theEntityManager.CreateEntity("playery",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playery")));
    for (auto p: players) {
        SOLDIER(p)->player = humanPlayer;
    }

    background = theEntityManager.CreateEntity("background",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("background"));

    // static entities
    grid.autoAssignEntitiesToCell(walls);
    grid.autoAssignEntitiesToCell(objs);

    visibilityManager.toggleVisibility(false);
    visibilityManager.init(grid);

    banner = theEntityManager.CreateEntity("banner",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("ui/banner"));
    turn = theEntityManager.CreateEntity("turn",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("ui/turn"));
    points = theEntityManager.CreateEntity("action_point",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("ui/action_point"));

    ANCHOR(banner)->parent =
    ANCHOR(turn)->parent =
    ANCHOR(points)->parent =
        camera;
    LOGI("PrototypeGame initialisation done.");
}

void PrototypeGame::quickInit() {
    sceneStateMachine.reEnterCurrentState();
}

void PrototypeGame::backPressed() {
}

void PrototypeGame::togglePause(bool) {

}

void PrototypeGame::tick(float dt) {
    static float accum = 5;
    accum +=dt;
    if (accum >= 5) {
        accum = 0;
        LOGI("Test file access");

#if SAC_EMSCRIPTEN
    #define TEST_FILE "/sac_temp/file1"
#else
    #define TEST_FILE "/tmp/file1"
#endif

        FILE* fd = fopen(TEST_FILE, "a");
        if (!fd) {
            LOGE("Meh ofstream is not good :'(");
        } else {
            // 'a' mode not properly handled by emscripten
            // we need to fseek ourselves
            fseek(fd, 0, SEEK_END);
            #define A "Write something\n"
            fwrite(A, 1, strlen(A), fd);

        }
        fclose(fd);

        fd = fopen(TEST_FILE, "r");
        if (!fd) {
            LOGE("Meh ifstream is not good :'(");
        } else {
            int rd = 0;
            do {
                char line[256];
                rd = fread(line, 1, 256, fd);
                line[rd] = '\0';
                if (rd)
                    LOGI("Read: '" << line << "'");
            } while (rd > 0);
        }
        fclose(fd);
#if SAC_EMSCRIPTEN
        const char* script = "" \
            "localStorage[\"sac_root\"] = window.JSON.stringify(FS.root.contents['sac_temp']);" \
            "localStorage[\"sac_nextInode\"] = window.JSON.stringify(FS.nextInode);";
        emscripten_run_script(script);
#endif
    }

    sceneStateMachine.update(dt);
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
