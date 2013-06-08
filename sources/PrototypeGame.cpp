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

#if SAC_INGAME_EDITORS
#include "util/PrototypeDebugConsole.h"
#endif

#define ZOOM 1


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
    Game::sacInit(windowW, windowH);

    SoldierSystem::CreateInstance();
    ActionSystem::CreateInstance();
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;

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
        }

        for (int i=1; i<3; ++i) {
            a.str("");
            a << "blueSoldier_" << i;
            Entity s = theEntityManager.CreateEntity(a.str(),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
            bEnnemies.push_back(s);
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
    sceneStateMachine.update(dt);
}

bool PrototypeGame::willConsumeBackEvent() {
    return false;
}
