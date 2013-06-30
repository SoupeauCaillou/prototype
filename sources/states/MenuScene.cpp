/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "base/StateMachine.h"
#include "Scenes.h"

#include "util/LevelLoader.h"

#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include "systems/TextSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "PrototypeGame.h"

#include <fstream>
#include <glm/gtx/vector_angle.hpp>

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    std::string choosenLevel;

    std::vector<Entity> levels;
    Entity levelEditorButton, levelEditorButtonContainer;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    std::pair<Entity, Entity> createTextAndContainer(bool mustBeInsertedInList, const std::string & name, const glm::vec2 & position) {
        //create the text
        Entity btn = theEntityManager.CreateEntity(name,
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("text"));
        TEXT(btn)->text = name;
        TRANSFORM(btn)->position = position;
        TEXT(btn)->show = true;

        //and its container
        Entity container = theEntityManager.CreateEntity(name + " container",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("text_container"));
        TRANSFORM(container)->position = TRANSFORM(btn)->position;
        TRANSFORM(container)->size = TRANSFORM(btn)->size;
        BUTTON(container)->enabled = true;

        if (mustBeInsertedInList) {
            levels.push_back(btn);
            levels.push_back(container);
        }

        return std::make_pair(btn, container);
    }

    void setup() {
        //add a level editor button
        auto pair = createTextAndContainer(false, "Level editor", glm::vec2(0.));
        levelEditorButton = pair.first;
        levelEditorButtonContainer = pair.second;
        TEXT(levelEditorButton)->show = BUTTON(levelEditorButtonContainer)->enabled = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        float sx = PlacementHelper::ScreenSize.x / 3.;
        float sy = PlacementHelper::ScreenSize.y / 3.;


        auto listOriginals = game->gameThreadContext->assetAPI->listAssetContent(".map", "levels");
        auto listUsers = game->gameThreadContext->assetAPI->listContent(game->gameThreadContext->assetAPI->getWritableAppDatasPath(), ".map");

        // +1 = levelEditorButton
        auto LEVEL_COUNT = listOriginals.size() + listUsers.size() + 1;


        //there must be an more elegant way to do that... but nvm
        float sqrtIs = sqrt(LEVEL_COUNT);
        int sqrtTot = (sqrtIs == (int)sqrtIs) ? sqrtIs : (int)sqrtIs + 1;


        int current = 0;

        for (auto file : listOriginals) {
            createTextAndContainer(true, file, glm::vec2(-sx + 2.f * sx * (current % sqrtTot) / sqrtTot,
             sy - 2.f * sy * (current / sqrtTot) / sqrtTot));
            ++current;
        }
        for (auto file : listUsers) {
            createTextAndContainer(true, file, glm::vec2(-sx + 2.f * sx * (current % sqrtTot) / sqrtTot,
             sy - 2.f * sy * (current / sqrtTot) / sqrtTot));
            ++current;
        }

        //update level editor btn
        TRANSFORM(levelEditorButton)->position = TRANSFORM(levelEditorButtonContainer)->position = glm::vec2(-sx + 2.f * sx * (current % sqrtTot) / sqrtTot, sy - 2.f * sy * (current / sqrtTot) / sqrtTot);
        TEXT(levelEditorButton)->show = BUTTON(levelEditorButtonContainer)->enabled = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(levelEditorButtonContainer)->clicked) {
            return Scene::LevelEditor;
        }

        for (unsigned i = 0; i < levels.size(); i += 2) {
            if (BUTTON(levels[i+1])->clicked) {
                choosenLevel = TEXT(levels[i])->text + ".map";

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
        for (auto it = levels.begin(); it != levels.end(); ++it) {
            theEntityManager.DeleteEntity(*it);
        }
        levels.clear();

        TEXT(levelEditorButton)->show = BUTTON(levelEditorButtonContainer)->enabled = false;

        if (to == Scene::Play) {
            bool isInAsset = LevelLoader::LoadFromFile(choosenLevel, game->gameThreadContext->assetAPI->loadAsset("levels/" + choosenLevel));
            if (! isInAsset) {
                LevelLoader::LoadFromFile(choosenLevel, game->gameThreadContext->assetAPI->loadFile(game->gameThreadContext->assetAPI->getWritableAppDatasPath() + choosenLevel));
            }
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
