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
    Entity startSolo;
#if SAC_NETWORK
    Entity startMulti, serverIp;
#endif

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        startSolo = theEntityManager.CreateEntity("startSolo",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("menu/startSolo"));
#if SAC_NETWORK
        startMulti = theEntityManager.CreateEntity("startMulti",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("menu/startMulti"));
        serverIp = theEntityManager.CreateEntity("serverIp",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("menu/serverIp"));
#endif
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        TEXT_RENDERING(startSolo)->show =
#if SAC_NETWORK
        TEXT_RENDERING(startMulti)->show =
        TEXT_RENDERING(serverIp)->show =
        BUTTON(startMulti)->enabled =
        RENDERING(startMulti)->show =
#endif
        BUTTON(startSolo)->enabled =
        RENDERING(startSolo)->show = true;

#if SAC_NETWORK
        game->gameThreadContext->keyboardInputHandlerAPI->askUserInput(game->serverIp, 15);
#endif
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
#if SAC_NETWORK
        std::string input;
        game->gameThreadContext->keyboardInputHandlerAPI->done(input);
        std::stringstream s;
        s << "Server: " << input;
        TEXT_RENDERING(serverIp)->text = s.str();
#endif
        if (BUTTON(startSolo)->clicked)
            return Scene::SelectCharacter;
#if SAC_NETWORK
        if (BUTTON(startMulti)->clicked)
            return Scene::Connecting;
#endif
        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
#if SAC_NETWORK
        TEXT_RENDERING(startMulti)->show =
        TEXT_RENDERING(serverIp)->show =
        RENDERING(startMulti)->show =
        RENDERING(serverIp)->show =
        BUTTON(startMulti)->enabled =
#endif
        TEXT_RENDERING(startSolo)->show =
        BUTTON(startSolo)->enabled =
        RENDERING(startSolo)->show = false;
#if SAC_NETWORK
        game->gameThreadContext->keyboardInputHandlerAPI->cancelUserInput();
#endif
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
