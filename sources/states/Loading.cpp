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

#include "base/PlacementHelper.h"
#include "base/TouchInputManager.h"

#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

#include "util/IntersectionUtil.h"

#include <glm/glm.hpp>

struct LoadingScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    // Scene variables
    Entity launchButton;
    Entity rocket;

    LoadingScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Setup internal var, states, ...
    void setup() {
        launchButton = theEntityManager.CreateEntity("launchButton",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("launchButton"));
        rocket = 0;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onEnter(Scene::Enum) override {
        BUTTON(launchButton)->enabled = true;
        
        if (rocket) {
            theEntityManager.DeleteEntity(rocket);
            rocket = 0;
        }
        rocket = theEntityManager.CreateEntity("rocket",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("rocket"));
        RENDERING(rocket)->texture = theRenderingSystem.loadTextureFile("fusee");
        RENDERING(rocket)->show =
            RENDERING(launchButton)->show = true;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override { 
        if (BUTTON(launchButton)->clicked) {
            return Scene::Launch;
        }
        if (theTouchInputManager.isTouched(0)) {
            const glm::vec2& point = theTouchInputManager.getTouchLastPosition(0);
            if (IntersectionUtil::pointRectangle(point, TRANSFORM(rocket)->position, TRANSFORM(rocket)->size)) {
                float ymax = TRANSFORM(rocket)->position.y + TRANSFORM(rocket)->size.y/2.f;
                float ymin = TRANSFORM(rocket)->position.y - TRANSFORM(rocket)->size.y/2.f;
                float perc = (point.y - ymin) / (ymax - ymin);
                ROCKET(rocket)->tankOccupied = perc;
                ROCKET(rocket)->tankOccupied *= ROCKET(rocket)->tankMax;
            }
        }

        return Scene::Loading;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        BUTTON(launchButton)->enabled = false;
        RENDERING(launchButton)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateLoadingSceneHandler(PrototypeGame* game) {
        return new LoadingScene(game);
    }
}
