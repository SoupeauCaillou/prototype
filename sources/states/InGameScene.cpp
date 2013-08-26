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
#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include "PrototypeGame.h"
#include "api/KeyboardInputHandlerAPI.h"
#include "api/NetworkAPI.h"
#include "api/KeyboardInputHandlerAPI.h"

#include "systems/OrcSystem.h"
#include "systems/ActionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/ParticuleSystem.h"

#include "util/IntersectionUtil.h"
#include <glm/gtx/rotate_vector.hpp>

struct InGameScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    bool fire;
    Entity orc;
    InGameScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        orc = ACTION(game->myOrcAction)->orc;

        fire = false;


        FileBuffer file = game->gameThreadContext->assetAPI->loadAsset("key_config.cfg");
        LOGF_IF(! file.data, "Unable to load key config file");

        DataFileParser dfp;
        dfp.load(file, "key_config.cfg");

        std::string templateKeyboard;
        dfp.get("", "template", &templateKeyboard);

        const std::map<std::string, int> keyNameToCodeValue = KeyboardInputHandler::keyNameToCodeValue;

        std::string key;
        dfp.get("Binding", "forward", &key);
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keyNameToCodeValue.at(templateKeyboard + "_" + key), [this] () -> void {
            ZSQD(orc)->directions.push_back(glm::vec2(0, 1));
        });
        dfp.get("Binding", "backward", &key);
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keyNameToCodeValue.at(templateKeyboard + "_" + key), [this] () -> void {
            ZSQD(orc)->directions.push_back(glm::vec2(0, -1));
        });
        dfp.get("Binding", "left", &key);
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keyNameToCodeValue.at(templateKeyboard + "_" + key), [this] () -> void {
            ZSQD(orc)->directions.push_back(glm::vec2(-1, 0));
        });
        dfp.get("Binding", "right", &key);
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyPress(keyNameToCodeValue.at(templateKeyboard + "_" + key), [this] () -> void {
            ZSQD(orc)->directions.push_back(glm::vec2(1, 0));
        });
        dfp.get("Binding", "fire", &key);
        game->gameThreadContext->keyboardInputHandlerAPI->registerToKeyRelease(keyNameToCodeValue.at(templateKeyboard + "_" + key), [this] () -> void {
            fire = true;
        });
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        // update server stuff
        if (!game->gameThreadContext->networkAPI->isConnectedToAnotherPlayer() ||
            game->gameThreadContext->networkAPI->amIGameMaster()) {

            theActionSystem.Update(dt);
            theOrcSystem.Update(dt);
        }

        if (fire) {
            // fire point
            Entity weapon = ORC(orc)->weapon;
            const glm::vec2& firePoint = TRANSFORM(weapon)->position +
                glm::rotate(glm::vec2(TRANSFORM(weapon)->size.x * 0.5f, 0.0f), TRANSFORM(weapon)->rotation);

            Entity bullet = theEntityManager.CreateEntityFromTemplate("ingame/bullet");
            TRANSFORM(bullet)->position = firePoint;
            PHYSICS(bullet)->addForce(
                Force(glm::rotate(glm::vec2(1000, 0), TRANSFORM(weapon)->rotation), glm::vec2(0.0f)),
                1.0f/60.0f);
            fire = false;
        }

        return Scene::InGame;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateInGameSceneHandler(PrototypeGame* game) {
        return new InGameScene(game);
    }
}
