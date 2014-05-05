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
#include "systems/AnchorSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "util/Random.h"
#include "PrototypeGame.h"

struct GameEndScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    GameEndScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() { }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    int activePlayer;
    std::vector<Entity> selected[4];
    std::map<Entity, Entity> highlight;

    void onEnter(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            if (game->playerActive[i] >= 0) {
                BUTTON(game->playerButtons[i])->enabled =
                    RENDERING(game->playerButtons[i])->show = true;
            }
        }
        activePlayer = -1;

        for (auto b: game->bees) {
            BUTTON(b)->enabled = true;
            PHYSICS(b)->mass = 0;
        }
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        for (int i = 0; i<4; i++) {
            if (BUTTON(game->playerButtons[i])->clicked) {
                activePlayer = i;
                break;
            }
        }

        if (activePlayer >= 0) {
            for (auto b: game->bees) {
                if (BUTTON(b)->clicked) {
                    auto& chosen = selected[activePlayer];

                    auto it = std::find(chosen.begin(), chosen.end(), b);
                    if (it == chosen.end()) {
                        Entity h = theEntityManager.CreateEntityFromTemplate("game/bee_highlight");
                        ANCHOR(h)->parent = b;
                        TRANSFORM(h)->size = TRANSFORM(ANCHOR(h)->parent)->size * 2.0f;
                        RENDERING(h)->color = game->playerColors[activePlayer + 1];
                        highlight[b] = h;
                    } else {
                        theEntityManager.DeleteEntity(highlight[b]);
                        highlight.erase(b);
                        chosen.erase(it);
                    }
                }
            }
        }

        return Scene::GameEnd;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
        for (int i=0; i<4; i++) {
            BUTTON(game->playerButtons[i])->enabled =
                RENDERING(game->playerButtons[i])->show = false;
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameEndSceneHandler(PrototypeGame* game) {
        return new GameEndScene(game);
    }
}
