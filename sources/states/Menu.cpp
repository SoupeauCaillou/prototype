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
#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include "util/IntersectionUtil.h"

#include "systems/LevelSystem.h"
#include "systems/BlockSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextRenderingSystem.h"

#include "PrototypeGame.h"

#include <glm/gtx/vector_angle.hpp>

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity currentLevel;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        glm::vec2 pointOfView(0.6222f, 0.0778f);
        // bug dans glm? notre point est à gauche de "l'origine", et un peu plus bas ... donc l'angle devrait etre -179.9 pas 179.9
        LOGF_IF(0.f <  glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(glm::vec2(-4.0966f, 0.0701f ) - pointOfView)),
            "Bug with GLM. You should fix it in glm/gtx/vector_angle.inl:36 -> change epsilon value to 0.00001 ");
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        LevelSystem::LoadFromFile("../../assetspc/level1.map");
#if SAC_DEBUG
        int i = 0;
        for (; i < PlacementHelper::ScreenHeight + 1; ++i) {
            Entity e = theEntityManager.CreateEntity("grid_line",
              EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_line"));
            TRANSFORM(e)->position.y = -.5 + PlacementHelper::ScreenHeight / 2. - i;
            TRANSFORM(e)->size.y = 1.;
        }

        for (int j = - 10; j <= 10; j += 2) {
            std::stringstream ss;
            ss << j;

            Entity e = theEntityManager.CreateEntity("grid_number_x " + ss.str(),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
            TEXT_RENDERING(e)->text = ss.str();
            TRANSFORM(e)->position.x = j;

            e = theEntityManager.CreateEntity("grid_number_y " + ss.str(),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
            TEXT_RENDERING(e)->text = ss.str();
            TRANSFORM(e)->position.y = j;
        }
#endif
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        theLevelSystem.Update(dt);
        theBlockSystem.Update(dt);

        static float lastChange = 0.f;

        //delete a block - right click over a block
        //add a block - right click too
        if (theTouchInputManager.wasTouched(1) && TimeUtil::GetTime() - lastChange > 1.) {
            lastChange = TimeUtil::GetTime();
            bool hasDeletedSomeOne = false;

            FOR_EACH_ENTITY(Block, e)
                TransformationComponent * tc = TRANSFORM(e);
                if (IntersectionUtil::pointRectangle(theTouchInputManager.getTouchLastPosition(1), tc->position, tc->size)) {
                    theEntityManager.DeleteEntity(e);
                    hasDeletedSomeOne = true;
                    break;
                }
            }


            if (! hasDeletedSomeOne) {
                Entity e = theEntityManager.CreateEntity("onClickBlock",
                  EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));

                RENDERING(e)->color = Color::random();
                TRANSFORM(e)->position = theTouchInputManager.getTouchLastPosition(1);
                TRANSFORM(e)->size = glm::vec2(glm::linearRand(1.f, 3.f));
            }
        }

        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
