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
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"

#include "systems/TransformationSystem.h"
#include "systems/BlockSystem.h"

#include "util/IntersectionUtil.h"
#include "util/drawVector.h"

#include "Scenes.h"


#include "PrototypeGame.h"

struct LevelEditorScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    std::list<Entity> drawVectorList;

    LevelEditorScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        static float lastChange = 0.f;
        static Entity firstSelectionned = 0;

        if (theTouchInputManager.wasTouched(1) && TimeUtil::GetTime() - lastChange > .4) {
            lastChange = TimeUtil::GetTime();

            auto mousePosition = theTouchInputManager.getTouchLastPosition(1);

            bool handled = false;

            FOR_EACH_ENTITY(Block, e)
                if (IntersectionUtil::pointRectangle(mousePosition, TRANSFORM(e)->position, TRANSFORM(e)->size)) {
                    if (firstSelectionned) {
                        if (firstSelectionned != e) {
                            drawVectorList.push_back(drawVector(TRANSFORM(firstSelectionned)->position, TRANSFORM(e)->position - TRANSFORM(firstSelectionned)->position));
                            RENDERING(firstSelectionned)->color = Color(1.f, 1.f, 1.f);
                        }
                        firstSelectionned = 0;
                    } else {
                        firstSelectionned = e;
                        RENDERING(firstSelectionned)->color = Color(1.f, 0.f, 0.f);
                    }
                    handled = true;
                    break;
                }
            }
            if (! handled) {
                Entity e = theEntityManager.CreateEntity("point",
                    EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
                TRANSFORM(e)->position = mousePosition;
                // RENDERING(e)->shape = Shape::Triangle;
            }
        }
        if (theTouchInputManager.wasTouched(0)) {
            return Scene::Menu;
        }

        return Scene::LevelEditor;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        while (drawVectorList.begin() != drawVectorList.end()) {
            theEntityManager.DeleteEntity(*drawVectorList.begin());
            drawVectorList.pop_front();
        }
    }

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateLevelEditorSceneHandler(PrototypeGame* game) {
        return new LevelEditorScene(game);
    }
}
