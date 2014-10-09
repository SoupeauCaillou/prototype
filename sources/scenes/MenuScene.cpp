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
#include "systems/AnchorSystem.h"
#include "systems/BlinkSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/CameraSystem.h"
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
    Entity cursor;
    std::vector<Entity> obstacles;
    std::vector<Entity> sheeps;
    std::vector<Entity> flowers;
    Entity farmer;
    float w[50];


    void onEnter(Scene::Enum) override {
        // flowers
        if (0) {
            std::vector<float> positions;
            positions.resize(100);
            Random::N_Floats(50, &positions[0], -10, 30);
            Random::N_Floats(50, &positions[50], -10, 10);

            for (int i=0; i<20; i++) {
                w[i] = 0.1;
                Entity f = theEntityManager.CreateEntity(HASH("flo/wer", 0x98f9748b));
                ADD_COMPONENT(f, Transformation);
                TRANSFORM(f)->size = glm::vec2(0.15);
                TRANSFORM(f)->position = glm::vec2(positions[i], positions[50 + i]);
                ADD_COMPONENT(f, Rendering);
                RENDERING(f)->show = true;
                RENDERING(f)->color = Color(0, 0.5, 0.3);
                flowers.push_back(f);
            }
        }

        obstacles.reserve(100);

        // add walls
        for (int i=0; i<4; i++) {
            Entity w = theEntityManager.CreateEntity(HASH("wall", 0x4b3afad3));
            ADD_COMPONENT(w, Transformation);
            ADD_COMPONENT(w, Rendering);
            obstacles.push_back(w);
        }
        TRANSFORM(obstacles[0])->size = TRANSFORM(obstacles[1])->size = glm::vec2(40, 1);
        TRANSFORM(obstacles[2])->size = TRANSFORM(obstacles[3])->size = glm::vec2(1, TRANSFORM(game->camera)->size.y);
        TRANSFORM(obstacles[0])->position = glm::vec2(10, TRANSFORM(game->camera)->size.y * 0.5 + 0.5);
        TRANSFORM(obstacles[1])->position = glm::vec2(10, -TRANSFORM(game->camera)->size.y * 0.5 - 0.5);
        TRANSFORM(obstacles[2])->position = glm::vec2(-10 - 0.5, 0);
        TRANSFORM(obstacles[3])->position = glm::vec2(30 + 0.5, 0);

        for (int i=0; i<30; i++) {
            Entity o = theEntityManager.CreateEntity(HASH("obstacle/o", 0x9082b3a8));
            ADD_COMPONENT(o, Transformation);
            TRANSFORM(o)->position = glm::vec2(
                Random::Float(-5, 30),
                Random::Float(-7, 7));
            TRANSFORM(o)->size = glm::vec2(
                Random::Float(0.75, 2.5),
                Random::Float(0.75, 1.5));
            TRANSFORM(o)->rotation = Random::Float(0, 6);
            ADD_COMPONENT(o, Rendering);
            RENDERING(o)->show = true;
            RENDERING(o)->flags = 1;

            float c = Random::Float(0.7, 1.0);
            RENDERING(o)->color = Color(c, c, c);
            RENDERING(o)->texture = HASH("bush", 0x93b23991);
            obstacles.push_back(o);

            Entity shadow = theEntityManager.CreateEntity(HASH("obstacle/shadow", 0xee4a9e9b));
            ADD_COMPONENT(shadow, Transformation);
            *TRANSFORM(shadow) = *TRANSFORM(o);
            TRANSFORM(shadow)->size *= 0.9;
            TRANSFORM(shadow)->position += glm::vec2(0.22, -0.18);
            TRANSFORM(shadow)->z -= 0.1;
            ADD_COMPONENT(shadow, Rendering);
            RENDERING(shadow)->flags = 1;
            RENDERING(shadow)->color = Color(0, 0, 0, 0.2);
            RENDERING(shadow)->show = true;
            RENDERING(shadow)->texture = HASH("bush", 0x93b23991);
        }
        cursor = theEntityManager.CreateEntity(HASH("cursor", 0x20716820));
        ADD_COMPONENT(cursor, Transformation);
        TRANSFORM(cursor)->size = glm::vec2(0.6f);
        TRANSFORM(cursor)->shape = Shape::Triangle;
        ADD_COMPONENT(cursor, Blink);
        BLINK(cursor)->enabled = true;
        BLINK(cursor)->visibleDuration = 1;
        BLINK(cursor)->hiddenDuration = 0.5;
        ADD_COMPONENT(cursor, Rendering);
        RENDERING(cursor)->color = Color(0.3, 0.5, 0.2);
        RENDERING(cursor)->show = true;

        CAMERA(game->camera)->clearColor = Color(1.0f, .968, 0.882);

        sheeps.reserve(3);
        int sheepCount = 5;
        int obs = obstacles.size();
        for (int i=0; i<sheepCount; i++) {
            Entity test = theEntityManager.CreateEntity(HASH("test", 0xe5145a4));
            ADD_COMPONENT(test, Transformation);
            TRANSFORM(test)->position = glm::vec2(Random::Float(-10, -6), Random::Float(-6, 6));
            TRANSFORM(test)->size.x = 1.3;
            ADD_COMPONENT(test, Rendering);
            RENDERING(test)->show = true;
            RENDERING(test)->flags = 1;
            RENDERING(test)->texture = HASH("mouton-bleu", 0x301aa388);
            ADD_COMPONENT(test, Physics);
            PHYSICS(test)->mass = 1.0f;
            PHYSICS(test)->instantRotation = 1;
            ADD_COMPONENT(test, AutonomousAgent);
            AUTONOMOUS(test)->maxSpeed = 4.0f;
            AUTONOMOUS(test)->maxForce = 30;
            AUTONOMOUS(test)->dangerThreshold = 0.25;
            AUTONOMOUS(test)->seek.entities = &flowers[0];
            AUTONOMOUS(test)->seek.count = 0;//flowers.size();
            AUTONOMOUS(test)->seek.weight = w;

            AUTONOMOUS(test)->flee.radius = 2;
            //AUTONOMOUS(test)->flee.target = toAvoid;
            AUTONOMOUS(test)->avoid.entities = &obstacles[0];
            AUTONOMOUS(test)->avoid.count = obs + sheepCount; // farmer => + 1;
            AUTONOMOUS(test)->group.count = sheepCount;
            AUTONOMOUS(test)->group.neighborRadius = 6;

            AUTONOMOUS(test)->wander.distance = 3;
            AUTONOMOUS(test)->wander.radius = 1;//3;
            AUTONOMOUS(test)->wander.jitter = 1;
            AUTONOMOUS(test)->wander.pauseDuration = Interval<float>(1, 2);
            obstacles.push_back(test);
            sheeps.push_back(test);

            Entity shadow = theEntityManager.CreateEntity(HASH("shadow", 0x862b0c63));
            ADD_COMPONENT(shadow, Transformation);
            *TRANSFORM(shadow) = *TRANSFORM(test);
            TRANSFORM(shadow)->size *= 0.9;
            ADD_COMPONENT(shadow, Anchor);
            ANCHOR(shadow)->position = glm::vec2(0.15, -0.12);
            ANCHOR(shadow)->z = -0.1;
            ANCHOR(shadow)->parent= test;
            ADD_COMPONENT(shadow, Rendering);
            RENDERING(shadow)->flags = 1;
            RENDERING(shadow)->color = Color(0, 0, 0, 0.2);
            RENDERING(shadow)->show = true;
            RENDERING(shadow)->texture = HASH("mouton-bleu", 0x301aa388);
        }

        {
            farmer = theEntityManager.CreateEntity(HASH("fermier", 0x2698fd15));
            ADD_COMPONENT(farmer, Transformation);
            TRANSFORM(farmer)->position = glm::vec2(0.0f);
            TRANSFORM(farmer)->size = glm::vec2(1.0);
            ADD_COMPONENT(farmer, Rendering);
            RENDERING(farmer)->show = true;
            RENDERING(farmer)->flags = 1;
            RENDERING(farmer)->texture = HASH("fermier", 0x2698fd15);
            ADD_COMPONENT(farmer, Physics);
            PHYSICS(farmer)->mass = 1.0f;
            PHYSICS(farmer)->instantRotation = 1;
            ADD_COMPONENT(farmer, AutonomousAgent);
            AUTONOMOUS(farmer)->maxSpeed = 6.0f;
            AUTONOMOUS(farmer)->maxForce = 40;
            AUTONOMOUS(farmer)->seek.entities = &cursor;
            AUTONOMOUS(farmer)->dangerThreshold = 0.5;
            AUTONOMOUS(farmer)->seek.count = 1;

            AUTONOMOUS(farmer)->avoid.entities = &obstacles[0];
            AUTONOMOUS(farmer)->avoid.count = obs + sheepCount + 1;

            obstacles.push_back(farmer);

            Entity shadow = theEntityManager.CreateEntity(HASH("shadow", 0x862b0c63));
            ADD_COMPONENT(shadow, Transformation);
            *TRANSFORM(shadow) = *TRANSFORM(farmer);
            TRANSFORM(shadow)->size *= 0.9;
            ADD_COMPONENT(shadow, Anchor);
            ANCHOR(shadow)->position = glm::vec2(0.15, -0.12);
            ANCHOR(shadow)->z = -0.1;
            ANCHOR(shadow)->parent= farmer;
            ADD_COMPONENT(shadow, Rendering);
            RENDERING(shadow)->flags = 1;
            RENDERING(shadow)->color = Color(0, 0, 0, 0.2);
            RENDERING(shadow)->show = true;
            RENDERING(shadow)->texture = HASH("fermier", 0x2698fd15);
        }

        for (int i=0; i<sheepCount; i++) {
            AUTONOMOUS(sheeps[i])->group.entities = &sheeps[0];
            AUTONOMOUS(sheeps[i])->flee.target = farmer;
            AUTONOMOUS(sheeps[i])->flee.radius = 3;
        }

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

        if (!flowers.empty()) {
            if (Random::Float() > 0.9) {
                int i = Random::Int(0, flowers.size() - 1);
                TRANSFORM(flowers[i])->position.x = Random::Float(-10, 30);
                TRANSFORM(flowers[i])->position.y = Random::Float(-10, 10);
            }
        }

        glm::vec2 c(0.0f);
        for (auto s: sheeps) c+= TRANSFORM(s)->position / (float)sheeps.size();
        TRANSFORM(game->camera)->position.x = glm::max(0.0f, glm::min(20.0f, TRANSFORM(farmer)->position.x));

        if (theTouchInputManager.wasTouched()) {
            TRANSFORM(cursor)->position = theTouchInputManager.getTouchLastPosition();
        }

        if (PHYSICS(farmer)->linearVelocity.x < 0) {
            RENDERING(farmer)->flags |= RenderingFlags::MirrorHorizontal;
        } else {
            RENDERING(farmer)->flags &= ~RenderingFlags::MirrorHorizontal;
        }

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
