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
#include <glm/gtx/rotate_vector.hpp>

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

        PLANE(plane2)->owner = player2;
        DCA(dca2)->owner = player2;
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
                static float lastTouch = 0;
                float ti = TimeUtil::GetTime();
                if (ti - lastTouch > 1) {
                    glm::vec2 cursorPosition = theTouchInputManager.getTouchLastPosition(1);
                    if (IntersectionUtil::pointRectangle(cursorPosition, TRANSFORM(p)->worldPosition, TRANSFORM(p)->size)) {
                        LOGW("Soldier '" << theEntityManager.entityName(p) << p << "' is dead");
                        PARATROOPER(p)->dead = true;
                    }
                    if (PARATROOPER(p)->parachute) {
                        Entity parachute = PARATROOPER(p)->parachute;
                        if (IntersectionUtil::pointRectangle(cursorPosition, TRANSFORM(parachute)->worldPosition, TRANSFORM(parachute)->size)) {
                            LOGI("adding damage for " << theEntityManager.entityName(parachute) << parachute
                                << "pos: " << TRANSFORM(p)->worldPosition << " size:" << TRANSFORM(parachute)->size
                                << "cursor: " << cursorPosition
                                << " at " << cursorPosition - TRANSFORM(parachute)->worldPosition + TRANSFORM(parachute)->worldPosition / 2.f);
                            PARACHUTE(parachute)->damages.push_back(cursorPosition - TRANSFORM(parachute)->worldPosition + TRANSFORM(parachute)->size / 2.f);
                        }

                        Entity hole = theEntityManager.CreateEntity("hole", EntityType::Volatile,
                             theEntityManager.entityTemplateLibrary.load("hole"));
                        PARACHUTE(parachute)->holes.push_back(hole);
                        TRANSFORM(hole)->parent = parachute;
                        TRANSFORM(hole)->position = glm::rotate(cursorPosition - TRANSFORM(parachute)->worldPosition,
                            -TRANSFORM(parachute)->worldRotation);
                    }
                    lastTouch = ti;
                }
            }
            //already got a parachute. Oust!
            if (PARATROOPER(p)->parachute)
                continue;

            //clicking on a paratrooper
            if (BUTTON(p)->mouseOver ) {
                const std::string name = PLAYER(PARATROOPER(p)->owner)->id == 0 ? "parachute_g" : "parachute_b";
                //create a parachute
                Entity parachute = theEntityManager.CreateEntity(name,
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(name));
                TRANSFORM(parachute)->parent = p;
                PARATROOPER(p)->parachute = parachute;

                AUTO_DESTROY(p)->onDeletionCall = [] (Entity p) {
                    Entity parachute = PARATROOPER(p)->parachute;
                    if (parachute) {
                        //TRANSFORM(p)->parent = 0;
                        theEntityManager.DeleteEntity(PARACHUTE(parachute)->fils);
                        for(auto it: PARACHUTE(parachute)->holes) {
                            theEntityManager.DeleteEntity(it);
                        }
                        theEntityManager.DeleteEntity(parachute);
                    }
                };
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
