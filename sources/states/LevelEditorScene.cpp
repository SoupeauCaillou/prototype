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

#include "api/KeyboardInputHandlerAPI.h"

#include "systems/ButtonSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/LevelSystem.h"
#include "systems/BlockSystem.h"
#include "systems/SpotSystem.h"

#include "util/IntersectionUtil.h"
#include "util/DrawSomething.h"

#include "Scenes.h"

#include "PrototypeGame.h"
#include <fstream>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>


struct LevelEditorScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    Entity saveButton, goTryLevelButton, tip;

    Entity firstSelectionned;

    std::list<Entity> wallList;
    std::list<Entity> spotList;

    bool waitingForLevelName, shouldGoTryAfterInput;

    LevelEditorScene(PrototypeGame* game) : StateHandler<Scene::Enum>(), waitingForLevelName(false), shouldGoTryAfterInput(false) {
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

        tip = theEntityManager.CreateEntity("objective",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
        TRANSFORM(tip)->position = glm::vec2(-5, -3);
        TEXT_RENDERING(tip)->flags |= TextRenderingComponent::MultiLineBit;
        TEXT_RENDERING(tip)->charHeight = .5f;
        TEXT_RENDERING(tip)->color = Color(0., 0., 0.);
        TEXT_RENDERING(tip)->show = false;
        TRANSFORM(tip)->size.x = 10;
    }

    enum EnumTip {
        NoSpot,
        SelectLevelName,
        SelectedName,
        Default,
    };

    void selectTip(EnumTip e, const std::string & opt = "") {
        switch (e) {
            case NoSpot:
                TEXT_RENDERING(tip)->text = "You need to create one spot at least!\n\
(double select a white square -> it should became blue)";
                TEXT_RENDERING(tip)->color = Color(1., 0., 0.);
                break;
            case SelectLevelName:
                TEXT_RENDERING(tip)->text = "Please write level name...";
                TEXT_RENDERING(tip)->color = Color(0., 0., 0.);
                break;
            case SelectedName:
                TEXT_RENDERING(tip)->text =  "Level name is " + opt;
                TEXT_RENDERING(tip)->color = Color(0., 0., 0.);
                break;
            default:
                TEXT_RENDERING(tip)->text = "Left click: create a point. \n\
Select it and it will be red (selectionned).\n\
Then click again on it and it will be a Spot (blue)\n\
or click on another point and you will create a wall.";
                TEXT_RENDERING(tip)->color = Color(0., 0., 0.);
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        TEXT_RENDERING(tip)->show =
        TEXT_RENDERING(saveButton)->show = BUTTON(saveButton)->enabled =
        TEXT_RENDERING(goTryLevelButton)->show = BUTTON(goTryLevelButton)->enabled = true;

        selectTip(EnumTip::Default);

        firstSelectionned = 0;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (waitingForLevelName) {
            std::string userLevelName;

            //if the user pressed enter, save the file
            if (game->gameThreadContext->keyboardInputHandlerAPI->done(userLevelName)) {
                theLevelSystem.SaveInFile("/tmp/" + userLevelName + ".map", wallList, spotList);
                waitingForLevelName = false;

                selectTip(EnumTip::NoSpot);

                //if he pressed "go try!", then change scene
                if (shouldGoTryAfterInput) {
                    LevelSystem::currentLevelPath = "/tmp/" + userLevelName + ".map";
                    shouldGoTryAfterInput = false;
                    return Scene::Menu;
                }
            } else if (userLevelName.size() > 0) {
                //show current input...
                selectTip(EnumTip::SelectedName, userLevelName);
            }
        }

        if (BUTTON(saveButton)->clicked || BUTTON(goTryLevelButton)->clicked) {
            if (theSpotSystem.getAllComponents().size() == 0) {
                selectTip(EnumTip::NoSpot);
            } else {
                waitingForLevelName = true;
                game->gameThreadContext->keyboardInputHandlerAPI->askUserInput();

                selectTip(EnumTip::SelectLevelName);

                shouldGoTryAfterInput = BUTTON(goTryLevelButton)->clicked;
            }
        } else {
            static float lastChange = 0.f;
            if (theTouchInputManager.wasTouched(0) && TimeUtil::GetTime() - lastChange > .4) {
                lastChange = TimeUtil::GetTime();

                auto mousePosition = theTouchInputManager.getTouchLastPosition(0);

                bool handled = false;

                FOR_EACH_ENTITY(Spot, e)
                    //if we clicked a spot, cancel the thing
                    if (IntersectionUtil::pointRectangle(mousePosition, TRANSFORM(e)->position, TRANSFORM(e)->size)) {
                        if (firstSelectionned) {
                            RENDERING(firstSelectionned)->color = Color(1.f, 1.f, 1.f);
                        }
                        firstSelectionned = 0;

                        handled = true;
                        break;
                    }
                }


                FOR_EACH_ENTITY(Block, e)
                    //if we clicked a block
                    if (IntersectionUtil::pointRectangle(mousePosition, TRANSFORM(e)->position, TRANSFORM(e)->size)) {
                        //if we already had a block selectionned earlier
                        if (firstSelectionned) {
                            //and the second block is not the first one, then create a wall
                            if (firstSelectionned != e) {
                                wallList.push_back(Draw::DrawVec2("LevelEditor", TRANSFORM(firstSelectionned)->position, TRANSFORM(e)->position - TRANSFORM(firstSelectionned)->position, Color(.1f, .1f, .1f)));
                                RENDERING(firstSelectionned)->color = Color(1.f, 1.f, 1.f);
                            //else the block is a spot;
                            } else {
                                Entity spot = theEntityManager.CreateEntity("spot",
                                    EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("spot"));
                                TRANSFORM(spot)->position = TRANSFORM(e)->position;
                                spotList.push_back(spot);

                                //if there were previously wall(s) for this block, delete it(them)
                                for (auto wallIt = wallList.begin(); wallIt != wallList.end();) {
                                    auto tc = TRANSFORM(*wallIt);
                                    glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);

                                    if (glm::length2(tc->position + offset - TRANSFORM(spot)->position) < 0.1f
                                        || glm::length2(tc->position - offset - TRANSFORM(spot)->position) < 0.1f) {
                                        theEntityManager.DeleteEntity(*wallIt);
                                        wallIt = wallList.erase(wallIt);
                                    } else {
                                        ++wallIt;
                                    }

                                }
                                //remove the block since we replaced it with a spot
                                theEntityManager.DeleteEntity(e);
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
                if (handled) {
                    selectTip(EnumTip::Default);
                } else {
                    Entity e = theEntityManager.CreateEntity("point",
                        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
                    TRANSFORM(e)->position = mousePosition;
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

        while (wallList.begin() != wallList.end()) {
            theEntityManager.DeleteEntity(*wallList.begin());
            wallList.pop_front();
        }

        while (spotList.begin() != spotList.end()) {
            theEntityManager.DeleteEntity(*spotList.begin());
            spotList.pop_front();
        }
    }

    void onExit(Scene::Enum) override {
        TEXT_RENDERING(tip)->show =
        TEXT_RENDERING(saveButton)->show = BUTTON(saveButton)->enabled =
        TEXT_RENDERING(goTryLevelButton)->show = BUTTON(goTryLevelButton)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateLevelEditorSceneHandler(PrototypeGame* game) {
        return new LevelEditorScene(game);
    }
}
