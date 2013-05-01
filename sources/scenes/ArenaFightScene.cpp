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
#include <sstream>
#include <vector>
#include <iomanip>

#include "base/EntityManager.h"
#include "base/TouchInputManager.h"

#include "systems/ZSQDSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/DefWeaponSystem.h"
#include "systems/AutonomousAgentSystem.h"
#include <glm/gtx/compatibility.hpp>


#include "api/KeyboardInputHandlerAPI.h"

#include "PrototypeGame.h"

struct ArenaFightScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity fighter, swords[2];

    std::list<Entity> enemy;

    ArenaFightScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() override {
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        fighter = theEntityManager.CreateEntity("fighter",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("fighter"));

        //to be fixed properly..
        char keys[4] = { 'Z', 'S', 'Q', 'D' };
        //char keys[4] = { '/', 'U', 'A', 'I' };

        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keys[0],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(0., 1.)); }
        );
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keys[1],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(0., -1.)); }
        );
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keys[2],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(-1., 0)); }
        );
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keys[3],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(1., 0)); }
        );


        swords[0] = theEntityManager.CreateEntity("sword_l",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("sword"));
        swords[1] = theEntityManager.CreateEntity("sword_r",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("sword"));
        TRANSFORM(swords[0])->parent = TRANSFORM(swords[1])->parent = fighter;
        TRANSFORM(swords[0])->position.x = -TRANSFORM(swords[0])->position.x;
        DEF_WEAPON(swords[0])->ellipseAngleRange.x = glm::pi<float>() - DEF_WEAPON(swords[1])->ellipseAngleRange.y;
        DEF_WEAPON(swords[0])->ellipseAngleRange.y = glm::pi<float>() - DEF_WEAPON(swords[1])->ellipseAngleRange.x;
        RENDERING(swords[0])->color.r = 0;
   }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        DEF_WEAPON(swords[0])->active = true;
        DEF_WEAPON(swords[0])->attack = theTouchInputManager.isTouched(1);
        DEF_WEAPON(swords[0])->target = theTouchInputManager.getTouchLastPosition(0);

        if (glm::linearRand(0.0f, 1.0f) < 1 * dt) {
            Entity e = theEntityManager.CreateEntity("enemy",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("fighter_autonomous"));

            enemy.push_front(e);
        }

        return Scene::ArenaFight;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onExit(Scene::Enum) override {
        theEntityManager.DeleteEntity(fighter);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateArenaFightSceneHandler(PrototypeGame* game) {
        return new ArenaFightScene(game);
    }
}
