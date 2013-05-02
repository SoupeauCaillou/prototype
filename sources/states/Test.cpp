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


#include "PrototypeGame.h"

#include "systems/dcasystem.h"
#include "systems/parachutesystem.h"
#include "systems/planesystem.h"
#include "systems/paratroopersystem.h"

#include "base/EntityManager.h"
#include "base/StateMachine.h"
#include "base/TouchInputManager.h"

#include "systems/AutoDestroySystem.h"
#include "systems/AnimationSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

#include <glm/gtx/compatibility.hpp>

#include "Scenes.h"


#include <sstream>
#include <vector>
#include <iomanip>

struct TestScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity plane, dca;
    std::list<Entity> paratroopers;

    TestScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() override {
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        plane = theEntityManager.CreateEntity("plane",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("plane"));
        BUTTON(plane)->enabled = true;
        RENDERING(plane)->show = true;
        dca = theEntityManager.CreateEntity("dca",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("dca"));
        RENDERING(dca)->show = true;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        if (BUTTON(plane)->clicked)
        {
            Entity paratrooper = theEntityManager.CreateEntity("paratrooper",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("paratrooper"));
            BUTTON(paratrooper)->enabled = true;
            RENDERING(paratrooper)->show = true;
            TRANSFORM(paratrooper)->position = TRANSFORM(plane)->position;
            
            // ADD_COMPONENT(paratrooper, AutoDestroy);
            // AUTO_DESTROY(paratrooper)->type = AutoDestroyComponent::OUT_OF_AREA;
            // AUTO_DESTROY(paratrooper)->params.area.x = AUTO_DESTROY(paratrooper)->params.area.y = 0;
            // AUTO_DESTROY(paratrooper)->params.area.w = TRANSFORM(game->camera)->size.x;
            // AUTO_DESTROY(paratrooper)->params.area.h = TRANSFORM(game->camera)->size.y;

            paratroopers.push_back(paratrooper);
        }

        for (auto& p : paratroopers) {
            if (BUTTON(p)->clicked) {
                PARACHUTE(p)->frottement = 1;
                PARACHUTE(p)->enable = true;
                RENDERING(p)->color = Color(1, 1, 1, 1);
            }    
        }

        return Scene::Test;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateTestSceneHandler(PrototypeGame* game) {
        return new TestScene(game);
    }
}
