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

#include "util/IntersectionUtil.h"

#include "systems/LevelSystem.h"
#include "systems/BlockSystem.h"
#include "systems/TransformationSystem.h"

#include "PrototypeGame.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity currentLevel;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        LevelSystem::LoadFromFile("../../assetspc/level1.map");
        //currentLevel = theEntityManager.CreateEntity("currentLevel",
         //   EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("level1"));
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        static float lastAdd = 0.f;
        //add a block - right click
        //delete a block - left click
        if (theTouchInputManager.wasTouched(0)) {
            FOR_EACH_ENTITY(Block, e)
                TransformationComponent * tc = TRANSFORM(e);
                if (IntersectionUtil::pointRectangle(theTouchInputManager.getTouchLastPosition(0), tc->position, tc->size)) {
                    theEntityManager.DeleteEntity(e);
                    break;
                }
            }
        }
        if (TimeUtil::GetTime() - lastAdd > 1. && theTouchInputManager.wasTouched(1)) {
            lastAdd = TimeUtil::GetTime();

            Entity e = theEntityManager.CreateEntity("onClickBlock",
              EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));

            RENDERING(e)->color = Color::random();
            TRANSFORM(e)->position = theTouchInputManager.getTouchLastPosition(1);
            TRANSFORM(e)->size = glm::vec2(glm::linearRand(1.f, 3.f));
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
