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

#include "systems/DCASystem.h"
#include "systems/ParachuteSystem.h"
#include "systems/PlaneSystem.h"
#include "systems/ParatrooperSystem.h"

#include "base/EntityManager.h"
#include "base/StateMachine.h"
#include "base/TouchInputManager.h"

#include "systems/AutoDestroySystem.h"
#include "systems/AnimationSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

#include "util/IntersectionUtil.h"

#include <glm/gtx/compatibility.hpp>
#include "glm/gtx/norm.hpp"

#include "Scenes.h"


#include <sstream>
#include <vector>
#include <iomanip>

struct TestScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity plane1, plane2, dca1, dca2;

    TestScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() override {
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        plane1 = theEntityManager.CreateEntity("plane1",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("plane1"));
        plane2 = theEntityManager.CreateEntity("plane2",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("plane2"));
        dca1 = theEntityManager.CreateEntity("dca_player1",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("DCA_player1"));
        dca2 = theEntityManager.CreateEntity("dca_player2",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("DCA_player2"));
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {

        if (theTouchInputManager.isTouched()) {
            glm::vec2 cursorPosition = theTouchInputManager.getTouchLastPosition();
            DCA(dca1)->targetPoint = cursorPosition;
            DCA(dca2)->targetPoint = cursorPosition;
        }

        FOR_EACH_ENTITY_COMPONENT(Plane, p, pc)
            //clicking on a plane
            if (BUTTON(p)->clicked) {
                pc->dropOne = true;
            }
        }

        FOR_EACH_ENTITY(Paratrooper, p)
            //already got a parachute. Oust!
            if (TRANSFORM(p)->parent)
                continue;

            //clicking on a paratrooper
            if (BUTTON(p)->clicked ) {
                //create a parachute
                Entity parachute = theEntityManager.CreateEntity("parachute",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("parachute"));
                TRANSFORM(parachute)->position = TRANSFORM(p)->worldPosition + glm::vec2(0.f, TRANSFORM(p)->size.y);
                TRANSFORM(p)->parent = parachute;
                TRANSFORM(p)->position = -glm::vec2(0.f, TRANSFORM(p)->size.y);
                TRANSFORM(p)->z = 0;

                //should be better done than that..
                PHYSICS(parachute)->linearVelocity = PHYSICS(p)->linearVelocity;
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
