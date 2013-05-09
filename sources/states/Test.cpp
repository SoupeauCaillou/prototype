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


#include "ParatroopersGame.h"

#include "systems/DCASystem.h"
#include "systems/ParachuteSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/PlaneSystem.h"
#include "systems/PlayerSystem.h"

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"
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
    ParatroopersGame* game;
    Entity plane1, plane2, dca1, dca2;
    Entity player1, player2;

    TestScene(ParatroopersGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() override {
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        player1 = theEntityManager.CreateEntity("player1",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("player1"));
        player2 = theEntityManager.CreateEntity("player2",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("player2"));

        plane1 = theEntityManager.CreateEntity("plane1",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("plane1"));
        plane2 = theEntityManager.CreateEntity("plane2",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("plane2"));
        dca1 = theEntityManager.CreateEntity("dca1",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("DCA1"));
        dca2 = theEntityManager.CreateEntity("dca2",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("DCA2"));

        PLANE(plane1)->owner = player1;
        DCA(dca1)->owner = player1;
        std::cout << PLAYER(player1)->playerColor << std::endl;
        RENDERING(plane1)->color = RENDERING(dca1)->color = PLAYER(player1)->playerColor;

        PLANE(plane2)->owner = player2;
        DCA(dca2)->owner = player2;
        RENDERING(plane2)->color = RENDERING(dca2)->color = PLAYER(player2)->playerColor;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (theTouchInputManager.isTouched()) {
            glm::vec2 cursorPosition = theTouchInputManager.getTouchLastPosition();
            DCA(dca1)->targetPoint = cursorPosition;
            DCA(dca2)->targetPoint = cursorPosition;
            FOR_EACH_ENTITY_COMPONENT(Plane, p, pc)
                //clicking on a plane
                if (IntersectionUtil::pointRectangle(cursorPosition, TRANSFORM(p)->worldPosition, TRANSFORM(p)->size)) {
                    pc->dropOne = true;
                }
            }
        }


        FOR_EACH_ENTITY(Paratrooper, p)
            if (theTouchInputManager.isTouched(1)) {
                glm::vec2 cursorPosition = theTouchInputManager.getTouchLastPosition(1);
                if (IntersectionUtil::pointRectangle(cursorPosition, TRANSFORM(p)->worldPosition, TRANSFORM(p)->size)) {
                    LOGW("Soldier '" << theEntityManager.entityName(p) << p << "' is dead");
                    PARATROOPER(p)->dead = true;
                }
                if (TRANSFORM(p)->parent) {
                    Entity parent = TRANSFORM(p)->parent;
                    if (IntersectionUtil::pointRectangle(cursorPosition, TRANSFORM(parent)->position, TRANSFORM(parent)->size)) {
                        LOGI("adding damage for " << theEntityManager.entityName(parent) << parent
                            << "pos: " << TRANSFORM(p)->worldPosition << " size:" << TRANSFORM(parent)->size
                            << "cursor: " << cursorPosition
                            << " at " << cursorPosition - TRANSFORM(parent)->worldPosition + TRANSFORM(parent)->worldPosition / 2.f);
                        PARACHUTE(parent)->damages.push_back(cursorPosition - TRANSFORM(parent)->worldPosition + TRANSFORM(parent)->size / 2.f);
                    }
                }
            }
            //already got a parachute. Oust!
            if (TRANSFORM(p)->parent)
                continue;

            //clicking on a paratrooper
            if (BUTTON(p)->clicked ) {
                //create a parachute
                Entity parachute = theEntityManager.CreateEntity("parachute",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("parachute"));
                TRANSFORM(parachute)->position = TRANSFORM(p)->worldPosition + glm::vec2(0.f, .5 * (TRANSFORM(parachute)->size.y + TRANSFORM(p)->size.y));
                TRANSFORM(p)->parent = parachute;
                TRANSFORM(p)->position = -glm::vec2(0.f, .5 * (TRANSFORM(parachute)->size.y + TRANSFORM(p)->size.y));
                TRANSFORM(p)->z = 0;

                AUTO_DESTROY(p)->onDeletionCall = [] (Entity e) { TRANSFORM(e)->parent = 0; };
                AUTO_DESTROY(parachute)->onDeletionCall = theParachuteSystem.destroyParachute;

                //should be better done than that..
                PHYSICS(parachute)->linearVelocity = PHYSICS(p)->linearVelocity;
            }
        }

        LOGW_EVERY_N(500, "player 1 : " << PLAYER(player1)->score);
        LOGW_EVERY_N(500, "player 2 : " << PLAYER(player2)->score);

        return Scene::Test;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateTestSceneHandler(ParatroopersGame* game) {
        return new TestScene(game);
    }
}
