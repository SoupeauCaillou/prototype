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

#include "Scenes.h"

#include "PrototypeGame.h"

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/SheepSystem.h"

#include "base/TouchInputManager.h"
#include "base/StateMachine.h"

#include "util/IntersectionUtil.h"


#include <glm/gtx/vector_angle.hpp>

struct InGameScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity cursor;
    float timeElapsed;

    InGameScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        cursor = theEntityManager.CreateEntity("cursor");   
        ADD_COMPONENT(cursor, Transformation);
        ADD_COMPONENT(cursor, Rendering);
        RENDERING(cursor)->color = Color(1., 0., 0.);
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        RENDERING(cursor)->show = true;

        auto sheep = theAutonomousAgentSystem.RetrieveAllEntityWithComponent();
        for (auto s : sheep) {
            AUTONOMOUS(s)->fleeTarget = cursor;
        }

        timeElapsed = 0.f;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        timeElapsed += dt;

        TRANSFORM(cursor)->position = theTouchInputManager.getTouchLastPosition();

        int sheepArrivedAtEnd = 0;
        auto sheep = theSheepSystem.RetrieveAllEntityWithComponent();
        for (auto s : sheep) {
            if (PHYSICS(s)->linearVelocity != glm::vec2(0.)) {
                TRANSFORM(s)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), 
                    glm::normalize(PHYSICS(s)->linearVelocity));
            }

            sheepArrivedAtEnd += IntersectionUtil::pointRectangle(TRANSFORM(s)->position, 
                TRANSFORM(game->levelLoader.arrivalZone)) ? 1 : 0;
        }


        //test win / lose conditions
        //time limit
        if (timeElapsed > game->levelLoader.objectiveTimeLimit) {
            LOGI("you lost! Time limit reached (" << timeElapsed << '>' << game->levelLoader.objectiveTimeLimit << ")");
            return Scene::GameEnd;
        // survival limit
        } else if ((int)sheep.size() < game->levelLoader.objectiveSurvived) {
            LOGI("you lost! Too many sheep died (" << sheep.size() << '<' << game->levelLoader.objectiveSurvived << ")");
            return Scene::GameEnd;
        // reached objective count
        } else if (sheepArrivedAtEnd >= game->levelLoader.objectiveArrived) {
            LOGI("you win! Time left: " << game->levelLoader.objectiveTimeLimit - timeElapsed);
            return Scene::GameEnd;
        }
        return Scene::InGame;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
        RENDERING(cursor)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateInGameSceneHandler(PrototypeGame* game) {
        return new InGameScene(game);
    }
}
