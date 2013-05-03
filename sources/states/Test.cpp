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

#include "util/IntersectionUtil.h"

#include <glm/gtx/compatibility.hpp>
#include "glm/gtx/norm.hpp"

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
        dca = theEntityManager.CreateEntity("dca",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("DCA"));
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {

        if (theTouchInputManager.isTouched()) {
            glm::vec2 point = theTouchInputManager.getTouchLastPosition();
            DCA(dca)->targetPoint = point;

            TransformationComponent *ptc = TRANSFORM(plane);
            if (IntersectionUtil::pointRectangle(point, ptc->position, ptc->size)) {
                Entity paratrooper = thePlaneSystem.paratrooperJump(plane);
                if (paratrooper)
                    paratroopers.push_back(paratrooper);
            }

            for (auto& p : paratroopers) {
                if (IntersectionUtil::pointRectangle(point, TRANSFORM(p)->position, TRANSFORM(p)->size)) {
                    PARACHUTE(p)->frottement = 1;
                    PARACHUTE(p)->enable = true;
                    RENDERING(p)->color = Color(1, 1, 1, 1);
                }
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
