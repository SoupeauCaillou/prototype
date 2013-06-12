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

#include "util/LevelLoader.h"

#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include "systems/TextRenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "PrototypeGame.h"


struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    std::string choosenLevel;

    std::vector<Entity> levels;
    Entity levelEditor;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        auto LEVEL_COUNT = game->gameThreadContext->assetAPI->listContent(".map", "levels").size();

        //inside sqrt '+1' is for levelEditor btn
        int sqrtTot = (int)sqrt(LEVEL_COUNT + 1) + 1;

        float sx = PlacementHelper::ScreenWidth / 3.;
        float sy = PlacementHelper::ScreenHeight / 3.;

        for (unsigned i = 0; i < LEVEL_COUNT; ++i) {
            std::stringstream ss;
            ss << "Level " << i + 1;

            Entity e = theEntityManager.CreateEntity(ss.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("action_button"));

            TEXT_RENDERING(e)->text = ss.str();

            TRANSFORM(e)->position = glm::vec2(-sx + 2.f * sx * (i % sqrtTot) / sqrtTot, sy - 2.f * sy * (i / sqrtTot) / sqrtTot);
            levels.push_back(e);
        }

        //finally, add a level editor button
        levelEditor = theEntityManager.CreateEntity("Level editor",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("action_button"));
        TEXT_RENDERING(levelEditor)->text = "Level editor";
        TRANSFORM(levelEditor)->position = glm::vec2(-sx + 2.f * sx * (LEVEL_COUNT % sqrtTot) / sqrtTot, sy - 2.f * sy * (LEVEL_COUNT / sqrtTot) / sqrtTot);
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        for (auto e : levels) {
            TEXT_RENDERING(e)->show = BUTTON(e)->enabled = true;
        }
        TEXT_RENDERING(levelEditor)->show = BUTTON(levelEditor)->enabled = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(levelEditor)->clicked) {
            return Scene::LevelEditor;
        }

        for (unsigned i = 0; i < levels.size(); ++i) {
            Entity e = levels[i];

            if (BUTTON(e)->clicked) {
                std::stringstream ss;
                ss << "levels/level" << i + 1 << ".map";
                choosenLevel = ss.str();

                return Scene::Play;
            }
        }
        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum to) override {
        for (auto e : levels) {
            TEXT_RENDERING(e)->show = BUTTON(e)->enabled = false;
        }
        TEXT_RENDERING(levelEditor)->show = BUTTON(levelEditor)->enabled = false;

        if (to == Scene::Play) {
            LevelLoader::LoadFromFile(choosenLevel, game->gameThreadContext->assetAPI->loadAsset(choosenLevel));
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
