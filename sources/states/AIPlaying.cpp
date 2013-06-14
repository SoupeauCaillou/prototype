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
#include "PrototypeGame.h"
#include "Scenes.h"
#include "CameraMoveManager.h"
#include "systems/TransformationSystem.h"
#include "systems/SoldierSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/ActionSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/RenderingSystem.h"
#include <glm/gtx/compatibility.hpp>

struct AIPlayingScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    // Scene variables

    AIPlayingScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~AIPlayingScene() {}

    // Setup internal var, states, ...
    void setup() override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onEnter(Scene::Enum) override {

    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
#if 0
        // How many action points left
        std::vector<Entity> units;
        theUnitAISystem.forEachECDo([&units] (Entity e, UnitAIComponent* uc) -> void {
            if (uc->active && uc->ready && !uc->preferedActions.empty()) {
                // is next action doable ?
                if (SOLDIER(e)->actionPointsLeft >= ActionSystem::ActionCost(ACTION(uc->preferedActions.front())->type)) {
                    units.push_back(e);
                }
            }
        });

        if (units.empty())
            return Scene::EndTurn;

        // Random AI FTW!
        int index = (int)glm::linearRandom(0.0f, (float)units.size());
#endif
        return Scene::AIPlaying;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateAIPlayingSceneHandler(PrototypeGame* game) {
        return new AIPlayingScene(game);
    }
}
