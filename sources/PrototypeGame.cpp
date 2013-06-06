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
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "base/TouchInputManager.h"
#include "base/EntityManager.h"
#include "base/TimeUtil.h"
#include "base/PlacementHelper.h"

#include "util/IntersectionUtil.h"

#include "api/StorageAPI.h"
#include "api/NetworkAPI.h"

#include "systems/BlockSystem.h"
#include "systems/SpotSystem.h"
#include "systems/LevelSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/TaskAISystem.h"
#include "systems/MusicSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/CameraSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/GraphSystem.h"

#define ZOOM 1


PrototypeGame::PrototypeGame() : Game() {
    sceneStateMachine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(this), "Scene::Logo");
    sceneStateMachine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(this), "Scene::Menu");
    sceneStateMachine.registerState(Scene::LevelEditor, Scene::CreateLevelEditorSceneHandler(this), "Scene::LevelEditor");
}

bool PrototypeGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        case ContextAPI::Sound:
            return true;
        default:
            return false;
    }
}

void PrototypeGame::sacInit(int windowW, int windowH) {
    LOGI("SAC engine initialisation begins...");
    Game::sacInit(windowW, windowH);
    PlacementHelper::GimpWidth = 0;
    PlacementHelper::GimpHeight = 0;
    LOGI("SAC engine initialisation done.");
}

void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    LevelSystem::CreateInstance();
    BlockSystem::CreateInstance();
    SpotSystem::CreateInstance();


#if SAC_DEBUG
    sceneStateMachine.setup(Scene::Menu);
#else
    sceneStateMachine.setup(Scene::Logo);
#endif

    // default camera
    camera = theEntityManager.CreateEntity("camera1");
    ADD_COMPONENT(camera, Transformation);
    TRANSFORM(camera)->size = glm::vec2(theRenderingSystem.screenW * ZOOM, theRenderingSystem.screenH * ZOOM);
    TRANSFORM(camera)->position = glm::vec2(0, 0);
    TRANSFORM(camera)->z = 1;
    ADD_COMPONENT(camera, Camera);
    CAMERA(camera)->enable = true;
    CAMERA(camera)->order = 2;
    CAMERA(camera)->id = 0;
    CAMERA(camera)->clearColor = Color(125.0/255, 150./255.0, 0.);

#if SAC_DEBUG
    //create a grid
    int i = 0;
    for (; i < PlacementHelper::ScreenHeight + 1; ++i) {
        Entity e = theEntityManager.CreateEntity("grid_line",
          EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_line"));
        TRANSFORM(e)->position.y = -.5 + PlacementHelper::ScreenHeight / 2. - i;
        TRANSFORM(e)->size.y = 1.;
    }

    for (int j = - 10; j <= 10; j += 2) {
        std::stringstream ss;
        ss << j;

        Entity e = theEntityManager.CreateEntity("grid_number_x " + ss.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
        TEXT_RENDERING(e)->text = ss.str();
        TRANSFORM(e)->position.x = j;

        e = theEntityManager.CreateEntity("grid_number_y " + ss.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
        TEXT_RENDERING(e)->text = ss.str();
        TRANSFORM(e)->position.y = j;
    }
#endif

    quickInit();
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
