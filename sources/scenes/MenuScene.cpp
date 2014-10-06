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
#include "systems/PhysicsSystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/RenderingSystem.h"
#include "base/TouchInputManager.h"
#include "api/NetworkAPI.h"
#include "api/KeyboardInputHandlerAPI.h"
#include "api/linux/NetworkAPILinuxImpl.h"
#include <SDL.h>
#include "util/Random.h"

#include "util/Draw.h"
#include "PrototypeGame.h"

#include <array>

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity playBtn, prevBtn, nextBtn, title;
    std::array<Entity, 3> btns;
    Entity objFinished, objTimeLimit, objDeadSheep;
    std::array<Entity, 3> objectives;

    std::vector<Entity> backgroundSheep;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>("Menu") {
        this->game = game;
    }

    void setup(AssetAPI*) override {
        /*
        btns[0] = playBtn = theEntityManager.CreateEntityFromTemplate("menu/playBtn");
        btns[1] = prevBtn = theEntityManager.CreateEntityFromTemplate("menu/prevBtn");
        btns[2] = nextBtn = theEntityManager.CreateEntityFromTemplate("menu/nextBtn");

        title = theEntityManager.CreateEntityFromTemplate("menu/title");

        objectives[0] = objFinished = theEntityManager.CreateEntityFromTemplate("menu/objectiveFinished");
        objectives[1] = objTimeLimit = theEntityManager.CreateEntityFromTemplate("menu/objectiveTimeLimit");
        objectives[2] = objDeadSheep = theEntityManager.CreateEntityFromTemplate("menu/objectiveDeadSheep");
        */
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
    Entity cursor, toAvoid, test;
    std::vector<Entity> obstacles;



    void onEnter(Scene::Enum) override {
        for (int i=0; i<24; i++) {
            Entity o = theEntityManager.CreateEntity(HASH("obstacle", 0x1f527e32));
            ADD_COMPONENT(o, Transformation);
            TRANSFORM(o)->position = glm::vec2(
                Random::Float(-10, 10),
                Random::Float(-7, 7));
            TRANSFORM(o)->size = glm::vec2(
                Random::Float(0.5, 2.5),
                Random::Float(0.5, 1.5));
            TRANSFORM(o)->rotation = Random::Float(0, 6);
            ADD_COMPONENT(o, Rendering);
            RENDERING(o)->show = true;
            RENDERING(o)->color = Color(1, 0, 0);
            obstacles.push_back(o);
        }
        cursor = theEntityManager.CreateEntity(HASH("cursor", 0x20716820));
        ADD_COMPONENT(cursor, Transformation);
        toAvoid = theEntityManager.CreateEntity(HASH("toAvoid", 0xbb936445));
        ADD_COMPONENT(toAvoid, Transformation);
        ADD_COMPONENT(toAvoid, Rendering);
        RENDERING(toAvoid)->color = Color(0, 0, 1);
        RENDERING(toAvoid)->show = true;
        test = theEntityManager.CreateEntity(HASH("test", 0xe5145a4));
        ADD_COMPONENT(test, Transformation);
        ADD_COMPONENT(test, Rendering);
        RENDERING(test)->show = true;
        ADD_COMPONENT(test, Physics);
        PHYSICS(test)->mass = 1.0f;
        PHYSICS(test)->maxSpeed = 5.0f;
        PHYSICS(test)->instantRotation = 1;
        ADD_COMPONENT(test, AutonomousAgent);
        AUTONOMOUS(test)->maxForce = 30;
        AUTONOMOUS(test)->dangerThreshold = 0.5;
        AUTONOMOUS(test)->seek.target = cursor;
        AUTONOMOUS(test)->flee.radius = 2;
        AUTONOMOUS(test)->flee.target = toAvoid;
        AUTONOMOUS(test)->avoid.entities = &obstacles[0];
        AUTONOMOUS(test)->avoid.count = (int)obstacles.size();

        /*
        updateLevelInformation();
        for (Entity s : backgroundSheep) {
            RENDERING(s)->show = true;
        }
        TEXT(title)->show = true;
        for (unsigned i = 0; i < btns.size(); ++i) {
            RENDERING(btns[i])->show = TEXT(btns[i])->show = BUTTON(btns[i])->enabled = true;
        }
        for (unsigned i = 0; i < objectives.size(); ++i) {
            RENDERING(objectives[i])->show = true;
        }
        */
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        Draw::Clear(HASH(__FILE__, 0x14211271));

        TRANSFORM(cursor)->position = theTouchInputManager.getOverLastPosition();
        if (theTouchInputManager.wasTouched()) {
            TRANSFORM(toAvoid)->position = theTouchInputManager.getTouchLastPosition();
        }
        Draw::Point(HASH(__FILE__, 0x14211271), TRANSFORM(cursor)->position);
        const glm::vec2& v = PHYSICS(test)->linearVelocity;
//        TRANSFORM(test)->rotation = glm::atan(v.y, v.x);

#if 0
        if (BUTTON(playBtn)->clicked) {
            LOGV(1, "Loading level: maps/" << game->levels[game->currentLevel] << ".ini");
            FileBuffer fb = game->gameThreadContext->assetAPI->loadAsset(
                "maps/" + game->levels[1 /* game->currentLevel*/] + ".ini");
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
#endif

        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        for (Entity s : backgroundSheep) {
            RENDERING(s)->show = false;
        }
        TEXT(title)->show = false;
        for (unsigned i = 0; i < btns.size(); ++i) {
            RENDERING(btns[i])->show = TEXT(btns[i])->show = BUTTON(btns[i])->enabled = false;
        }
        for (unsigned i = 0; i < objectives.size(); ++i) {
            RENDERING(objectives[i])->show = false;
        }

    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
