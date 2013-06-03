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

#include "systems/ButtonSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/BlockSystem.h"

#include "util/IntersectionUtil.h"
#include "util/drawVector.h"

#include "Scenes.h"

#include "PrototypeGame.h"
#include <fstream>
#include <glm/gtx/rotate_vector.hpp>


struct LevelEditorScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    Entity saveButton, goTryLevelButton;

    std::list<Entity> drawVectorList;

    LevelEditorScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        saveButton = theEntityManager.CreateEntity("save",
                    EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("action_button"));
        TRANSFORM(saveButton)->position.x = -9;
        TEXT_RENDERING(saveButton)->text = "save";

        goTryLevelButton = theEntityManager.CreateEntity("go_try_level",
                    EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("action_button"));
        TRANSFORM(goTryLevelButton)->position.x = -5;
        TEXT_RENDERING(goTryLevelButton)->text = "try it!";
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        TEXT_RENDERING(saveButton)->show = BUTTON(saveButton)->enabled =
        TEXT_RENDERING(goTryLevelButton)->show = BUTTON(goTryLevelButton)->enabled = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(saveButton)->clicked || BUTTON(goTryLevelButton)->clicked) {
             const auto filename = "/tmp/level_editor.map";
            //save in file each walls
            std::ofstream myfile (filename);
            LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

            for (auto wall : drawVectorList) {
                TransformationComponent * tc = TRANSFORM(wall);

                glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);
                myfile << tc->position - offset << " | " << tc->position + offset << "\n";
            }
            myfile.close();


            if (BUTTON(goTryLevelButton)->clicked) {
                return Scene::Menu;
            }
        } else {
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
                            }
                            RENDERING(firstSelectionned)->color = Color(1.f, 1.f, 1.f);
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
        }
        return Scene::LevelEditor;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        FOR_EACH_ENTITY(Block, e)
            theEntityManager.DeleteEntity(e);
        }

        while (drawVectorList.begin() != drawVectorList.end()) {
            theEntityManager.DeleteEntity(*drawVectorList.begin());
            drawVectorList.pop_front();
        }
    }

    void onExit(Scene::Enum) override {
        TEXT_RENDERING(saveButton)->show = BUTTON(saveButton)->enabled =
        TEXT_RENDERING(goTryLevelButton)->show = BUTTON(goTryLevelButton)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateLevelEditorSceneHandler(PrototypeGame* game) {
        return new LevelEditorScene(game);
    }
}
