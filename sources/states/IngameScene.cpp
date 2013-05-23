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
#include "base/EntityManager.h"

#include "systems/InputSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/CollisionSystem.h"

#include "api/KeyboardInputHandlerAPI.h"
#include "api/StorageAPI.h"
#include "util/ScoreStorageProxy.h"

#include "PrototypeGame.h"

struct IngameScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity ground, torero, barriers[4];

    IngameScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        ground = theEntityManager.CreateEntity("ground",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("ingame/ground"));

        barriers[0] = theEntityManager.CreateEntity("barrier_n",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("ingame/barrier_n"));
        barriers[1] = theEntityManager.CreateEntity("barrier_s",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("ingame/barrier_s"));
        barriers[2] = theEntityManager.CreateEntity("barrier_e",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("ingame/barrier_e"));
        barriers[3] = theEntityManager.CreateEntity("barrier_w",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("ingame/barrier_w"));


        torero = theEntityManager.CreateEntity("torero",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("ingame/torero"));

        ANCHOR(game->camera)->parent = torero;

        Entity ball = theEntityManager.CreateEntity("ball",
                EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("ingame/ball"));

        theCollisionSystem.worldSize = glm::vec2(50);
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        INPUT(torero)->direction = glm::vec2(0);
        if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_LEFT)) {
            INPUT(torero)->direction.x = -1;
        } else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_RIGHT)) {
            INPUT(torero)->direction.x = 1;
        }
        if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_UP)) {
            INPUT(torero)->direction.y = 1;
        } else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_DOWN)) {
            INPUT(torero)->direction.y = -0.5;
        }
        INPUT(torero)->lateralMove = game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_LSHIFT);

        theInputSystem.Update(dt);
        return Scene::Ingame;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        theEntityManager.DeleteEntity(ground);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateIngameSceneHandler(PrototypeGame* game) {
        return new IngameScene(game);
    }
}
