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

#include <systems/TransformationSystem.h>
#include <systems/CameraSystem.h>
#include <systems/RenderingSystem.h>
#include <systems/PhysicsSystem.h>


#include "api/StorageAPI.h"
#include "util/ScoreStorageProxy.h"

#include "PrototypeGame.h"

struct ArenaFightScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity fighter, swords[2];

    ArenaFightScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {

    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        fighter = theEntityManager.CreateEntity("fighter",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("fighter"));
        swords[0] = theEntityManager.CreateEntity("sword_l",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("sword"));
        swords[1] = theEntityManager.CreateEntity("sword_r",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("sword"));
        TRANSFORM(swords[0])->position.x = -TRANSFORM(swords[0])->position.x;
        TRANSFORM(swords[0])->parent = TRANSFORM(swords[1])->parent = fighter;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        return Scene::ArenaFight;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onExit(Scene::Enum) override {
        theEntityManager.DeleteEntity(fighter);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateArenaFightSceneHandler(PrototypeGame* game) {
        return new ArenaFightScene(game);
    }
}
