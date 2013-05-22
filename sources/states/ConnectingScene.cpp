/*
	This file is part of RecursiveRunner.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	RecursiveRunner is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	RecursiveRunner is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "base/StateMachine.h"

#include "Scenes.h"
#include <sstream>
#include <vector>
#include <iomanip>

#include "base/EntityManager.h"
#include "base/ObjectSerializer.h"

#include <systems/AnchorSystem.h>
#include <systems/TransformationSystem.h>
#include <systems/ButtonSystem.h>
#include <systems/RenderingSystem.h>
#include <systems/TextRenderingSystem.h>
#include <systems/AutoDestroySystem.h>
#include <systems/PhysicsSystem.h>

#include "api/KeyboardInputHandlerAPI.h"
#include "api/StorageAPI.h"
#include "util/ScoreStorageProxy.h"

#include "PrototypeGame.h"

struct ConnectingScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity waitIcon, infoText, cancelBtn;

    ConnectingScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        waitIcon = theEntityManager.CreateEntity("waitIcon",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("connecting/waitIcon"));
        infoText = theEntityManager.CreateEntity("infoText",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("connecting/infoText"));
        cancelBtn = theEntityManager.CreateEntity("cancelBtn",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("connecting/cancelBtn"));
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        TEXT_RENDERING(infoText)->show =
        TEXT_RENDERING(cancelBtn)->show =
        BUTTON(cancelBtn)->enabled =
        RENDERING(waitIcon)->show =
        RENDERING(cancelBtn)->show = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        TRANSFORM(waitIcon)->rotation += 5 * dt;

        return Scene::Connecting;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        TEXT_RENDERING(infoText)->show =
        TEXT_RENDERING(cancelBtn)->show =
        BUTTON(cancelBtn)->enabled =
        RENDERING(waitIcon)->show =
        RENDERING(cancelBtn)->show = false;

    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateConnectingSceneHandler(PrototypeGame* game) {
        return new ConnectingScene(game);
    }
}
