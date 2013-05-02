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

#include "systems/SwordManSystem.h"
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

        //char keys[4] = { SDLK_z, SDLK_s, SDLK_q, SDLK_d};
        //char keys[4] = { SDLK_SLASH ,SDLK_u, SDLK_a, SDLK_i };
        char keys[4] = {25, 39, 38, 40 }; //maybe not the best way, but...nothing's better yet!
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPressPerScancode(keys[0],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(0., 1.)); }
        );
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPressPerScancode(keys[1],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(0., -1.)); }
        );
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPressPerScancode(keys[2],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(-1., 0)); }
        );
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPressPerScancode(keys[3],
            [this] () { ZSQD(fighter)->addDirectionVector(glm::vec2(1., 0)); }
        );
   }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        SwordManComponent* swmc = SWORD_MAN(fighter);
        DEF_WEAPON(swmc->hands[0])->active = true;
        DEF_WEAPON(swmc->hands[0])->attack = theTouchInputManager.isTouched(1);
        DEF_WEAPON(swmc->hands[0])->target = theTouchInputManager.getTouchLastPosition(0);

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
