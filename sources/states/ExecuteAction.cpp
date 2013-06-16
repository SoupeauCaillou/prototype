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
#include "systems/ActionSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/SoldierSystem.h"
#include "PrototypeGame.h"
#include "CameraMoveManager.h"
#include "systems/TransformationSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/VisionSystem.h"

struct ExecuteActionScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    // Scene variables


    ExecuteActionScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~ExecuteActionScene() {}

    // Setup internal var, states, ...
    void setup() override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreEnter(Scene::Enum) {}
    bool updatePreEnter(Scene::Enum, float) override {return true;}
    void onEnter(Scene::Enum) override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        theCameraMoveManager.update(dt, game->camera);

        unsigned countBefore = theActionSystem.getAllComponents().size();
        theActionSystem.Update(dt);
        unsigned countAfter = theActionSystem.getAllComponents().size();

        // Update UI
        std::stringstream ss2;
        ss2 << "AP left: " << PLAYER(game->humanPlayer)->actionPointsLeft;
        TEXT_RENDERING(game->points)->text = ss2.str();

        if (countAfter < countBefore) {
            theVisionSystem.Update(dt);
            // update visibility after action finished
            game->visibilityManager.updateVisibility(game->players);
        }
        if (countAfter == 0) {
            if (game->aiPlaying)
                return Scene::AIThinking;
            else
                return Scene::SelectAction;
        }

        return Scene::ExecuteAction;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}

    void onExit(Scene::Enum) override {
        game->grid.autoAssignEntitiesToCell(game->players);

        if (PLAYER(game->humanPlayer)->actionPointsLeft == 0)
            TEXT_RENDERING(game->banner)->color = Color(1, 0, 0);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateExecuteActionSceneHandler(PrototypeGame* game) {
        return new ExecuteActionScene(game);
    }
}
