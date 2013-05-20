/*
    This file is part of Prototype.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer

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

#include "systems/RocketSystem.h"

#include "systems/PhysicsSystem.h"

#include "glm/glm.hpp"

struct LaunchScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    // Scene variables
    float s;

    LaunchScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~LaunchScene() {}

    // Setup internal var, states, ...
    void setup() override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreEnter(Scene::Enum) {}
    bool updatePreEnter(Scene::Enum, float) override {return true;}
    void onEnter(Scene::Enum) override {
        FOR_EACH_ENTITY(Rocket, e) 
            //PHYSICS(e)->gravity =  glm::vec2(0.f, -0.981f);
        }
        s = 0;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        s += dt;
    	theRocketSystem.Update(dt);
    	FOR_EACH_ENTITY_COMPONENT(Rocket, e, rc) 
            if (s > 1)
                PHYSICS(e)->gravity =  glm::vec2(0.f, -0.981f);
    		if (rc->tankOccupied == 0 && PHYSICS(e)->linearVelocity.y < 0) {
    			return Scene::Score;
    		}
    	}
    	
        return Scene::Launch;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
    bool updatePreExit(Scene::Enum, float) override {return true;}
    void onExit(Scene::Enum) override {}
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateLaunchSceneHandler(PrototypeGame* game) {
        return new LaunchScene(game);
    }
}
