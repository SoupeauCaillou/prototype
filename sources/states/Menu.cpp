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

#include "CameraMoveManager.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity startSolo, startMulti, serverIp;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        startSolo = theEntityManager.CreateEntity("startSolo",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("menu/startSolo"));
        startMulti = theEntityManager.CreateEntity("startMulti",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("menu/startMulti"));
        serverIp = theEntityManager.CreateEntity("serverIp",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("menu/serverIp"));
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        TEXT_RENDERING(startSolo)->show =
        TEXT_RENDERING(startMulti)->show =
        TEXT_RENDERING(serverIp)->show =
        BUTTON(startSolo)->enabled =
        BUTTON(startMulti)->enabled =
        RENDERING(startSolo)->show =
        RENDERING(startMulti)->show = true;

        game->gameThreadContext->keyboardInputHandlerAPI->askUserInput(game->serverIp, 15);
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        std::string input;
        game->gameThreadContext->keyboardInputHandlerAPI->done(input);
        std::stringstream s;
        s << "Server: " << input;
        TEXT_RENDERING(serverIp)->text = s.str();
        if (BUTTON(startSolo)->clicked)
            return Scene::SelectCharacter;
        if (BUTTON(startMulti)->clicked)
            return Scene::Connecting;
        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        TEXT_RENDERING(startSolo)->show =
        TEXT_RENDERING(startMulti)->show =
        TEXT_RENDERING(serverIp)->show =
        BUTTON(startSolo)->enabled =
        BUTTON(startMulti)->enabled =
        RENDERING(startSolo)->show =
        RENDERING(startMulti)->show =
        RENDERING(serverIp)->show = false;

        game->gameThreadContext->keyboardInputHandlerAPI->cancelUserInput();
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
