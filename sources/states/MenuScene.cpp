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

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "api/NetworkAPI.h"
#include "api/linux/NetworkAPILinuxImpl.h"

#include "PrototypeGame.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity playBtn, prevBtn, nextBtn, title;
    std::array<Entity, 3> btns;
    Entity objFinished, objTimeLimit, objDeadSheep;
    std::array<Entity, 3> objectives;

    std::vector<Entity> backgroundSheep;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        btns[0] = playBtn = theEntityManager.CreateEntityFromTemplate("menu/playBtn");
        btns[1] = prevBtn = theEntityManager.CreateEntityFromTemplate("menu/prevBtn");
        btns[2] = nextBtn = theEntityManager.CreateEntityFromTemplate("menu/nextBtn");

        title = theEntityManager.CreateEntityFromTemplate("menu/title");

        objectives[0] = objFinished = theEntityManager.CreateEntityFromTemplate("menu/objectiveFinished");
        objectives[1] = objTimeLimit = theEntityManager.CreateEntityFromTemplate("menu/objectiveTimeLimit");
        objectives[2] = objDeadSheep = theEntityManager.CreateEntityFromTemplate("menu/objectiveDeadSheep");
    }


    void updateLevelInformation() {
        std::string & level = game->levels[game->currentLevel];

        TEXT(playBtn)->text = level;
        
        if ("1" == game->saveManager.getValue("level_finished_" + level)) {
            RENDERING(objFinished)->color = Color(1, 0, 0);
        } else {
            RENDERING(objFinished)->color = Color(0, 0, 0);
        }
        if ("1" == game->saveManager.getValue("level_time_done_" + level)) {
            RENDERING(objTimeLimit)->color = Color(0, 1, 0);
        } else {
            RENDERING(objTimeLimit)->color = Color(0, 0, 0);
        }
        if ("1" == game->saveManager.getValue("level_sheep_done_" + level)) {
            RENDERING(objDeadSheep)->color = Color(0, 0, 1);
        } else {
            RENDERING(objDeadSheep)->color = Color(0, 0, 0);
        }

        //calculate the number of sheep
        unsigned sheepCount = 0;
        for (auto & l : game->levels) {
            if ("1" == game->saveManager.getValue("level_finished_" + l)) {
                sheepCount++;
            }
            if ("1" == game->saveManager.getValue("level_time_done_" + l)) {
                sheepCount++;
            }
            if ("1" == game->saveManager.getValue("level_sheep_done_" + l)) {
                sheepCount++;
            }
        }

        auto b = PlacementHelper::ScreenSize;
        while (sheepCount > backgroundSheep.size()) {
            Entity s = theEntityManager.CreateEntityFromTemplate("menu/backgroundSheep");
            TRANSFORM(s)->position.x = glm::linearRand(- b.x * .5f, b.x * .5f);
            TRANSFORM(s)->position.y = glm::linearRand(- b.y * .5f, - b.y * .25f);
            backgroundSheep.push_back(s);
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onPreEnter(Scene::Enum) override {
        TEXT(title)->show = true;
        for (unsigned i = 0; i < btns.size(); ++i) {
            RENDERING(btns[i])->show = TEXT(btns[i])->show = BUTTON(btns[i])->enabled = true;
        }
        for (unsigned i = 0; i < objectives.size(); ++i) {
            RENDERING(objectives[i])->show = true;
        }
    }

    
    void onEnter(Scene::Enum) override {
        updateLevelInformation();
        for (Entity s : backgroundSheep) {
            RENDERING(s)->show = true;
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(playBtn)->clicked) {
            FileBuffer fb = game->gameThreadContext->assetAPI->loadAsset(
                "maps/" + game->levels[game->currentLevel] + ".ini");
            game->levelLoader.load(fb);
            return Scene::GameStart;
        }
        if (BUTTON(prevBtn)->clicked) {
            game->currentLevel = glm::max(0, game->currentLevel - 1);
            // game->currentLevel = (game->currentLevel - 1 + game->levels.size()) % game->levels.size();
            updateLevelInformation();
            
        }
        if (BUTTON(nextBtn)->clicked) {
            game->currentLevel = glm::min((int)game->levels.size() - 1, game->currentLevel + 1);
            // game->currentLevel = (game->currentLevel + 1) % game->levels.size();
            updateLevelInformation();
        }


        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        TEXT(title)->show = false;
        for (unsigned i = 0; i < btns.size(); ++i) {
            RENDERING(btns[i])->show = TEXT(btns[i])->show = BUTTON(btns[i])->enabled = false;
        }
        for (unsigned i = 0; i < objectives.size(); ++i) {
            RENDERING(objectives[i])->show = false;
        }
    }

    void onExit(Scene::Enum) override {
        for (Entity s : backgroundSheep) {
            RENDERING(s)->show = false;
        }

    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
