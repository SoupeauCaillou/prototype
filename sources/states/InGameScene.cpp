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

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/PhysicsSystem.h"

#include "base/TouchInputManager.h"
#include "base/StateMachine.h"

#include "Scenes.h"

#include <glm/gtx/vector_angle.hpp>

struct InGameScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity cursor;

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
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        TRANSFORM(cursor)->position = theTouchInputManager.getTouchLastPosition();

        auto sheep = theAutonomousAgentSystem.RetrieveAllEntityWithComponent();
        for (auto s : sheep) {
            if (PHYSICS(s)->linearVelocity != glm::vec2(0.)) {
                TRANSFORM(s)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), 
                    glm::normalize(PHYSICS(s)->linearVelocity));
            }
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
