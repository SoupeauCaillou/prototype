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

#include "CameraMoveManager.h"
#include "PrototypeGame.h"

#include "util/SpatialGrid.h"

#include "base/EntityManager.h"
#include "systems/PlayerSystem.h"
#include "util/SpatialGrid.h"
#include "systems/AutoDestroySystem.h"
#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/TransformationSystem.h"

#include <map>

#define GridSize 1.2f

struct SelectCharacterScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    SelectCharacterScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~SelectCharacterScene() {}

    // Setup internal var, states, ...
    void setup() override {

    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    bool updatePreEnter(Scene::Enum, float) override {return true;}
    void onPreEnter(Scene::Enum) override {
        for (auto wall: game->walls) {
            RENDERING(wall)-> show = true;
        }
        for (auto s: game->yEnnemies) {
            RENDERING(s)-> show = true;
        }

        game->visibilityManager.reset();
        for (auto p: game->players) {
            RENDERING(p)->show = true;

            game->visibilityManager.updateVisibility(
                game->grid,
                game->grid.positionToGridPos(TRANSFORM(p)->position),
                6);
        }
        for (auto o: game->objs) {
            RENDERING(o)->show = true;
        }
        for (auto s: game->bEnnemies) {
            RENDERING(s)->show = true;
        }
        RENDERING(game->background)->show = true;

        // quick test
        game->grid.doForEachCell([this] (const GridPos& p) -> void {
            std::list<Entity>& l = this->game->grid.getEntitiesAt(p);
            for (auto& e: l) {
                RENDERING(e)->show = true;
            }
        });

        game->grid.autoAssignEntitiesToCell(game->players);
        game->grid.autoAssignEntitiesToCell(game->yEnnemies);
        game->grid.autoAssignEntitiesToCell(game->bEnnemies);

        game->activeCharacter = 0;

        if (PLAYER(game->humanPlayer)->actionPointsLeft == 0)
            TEXT_RENDERING(game->banner)->color = Color(1, 0, 0);
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        if (theCameraMoveManager.update(dt, game->camera))
            return Scene::SelectCharacter;

        if (BUTTON(game->banner)->clicked)
            return Scene::EndTurn;

        for (auto p: game->players) {
            if (BUTTON(p)->clicked) {
                game->activeCharacter = p;
                return Scene::SelectAction;;
            }
        }

        static float debugAcc = 0;
        debugAcc += dt;
        if (0 && theTouchInputManager.isTouched(1) && debugAcc > 1) {
            debugAcc = 0;
            GridPos pos = game->grid.positionToGridPos(theTouchInputManager.getTouchLastPosition());
            std::map<int, std::vector<GridPos> > v = game->grid.movementRange(pos, 4);
            std::stringstream a;
            for (auto i: v) {
                a.str("");
                a << i.first;
                for (auto b: i.second) {
                    Entity e = theEntityManager.CreateEntity("t");
                    ADD_COMPONENT(e, Transformation);
                    TRANSFORM(e)->size = glm::vec2(1.f);
                    TRANSFORM(e)->z = 0.9;
                    ADD_COMPONENT(e, TextRendering);
                    TEXT_RENDERING(e)->color = Color(1,0,0,1);
                    TEXT_RENDERING(e)->text = a.str();
                    TEXT_RENDERING(e)->show = true;
                    TRANSFORM(e)->position = game->grid.gridPosToPosition(b);
                    ADD_COMPONENT(e, AutoDestroy);
                    AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;
                    AUTO_DESTROY(e)->params.lifetime.freq.value = 3;
                    AUTO_DESTROY(e)->params.lifetime.map2AlphaTextRendering = true;
                }
            }
            std::vector<GridPos> m = game->grid.ringFinder(pos, 6, false);
            std::vector<GridPos> s = game->grid.viewRange(pos, 6);

            for (auto i: m) {
                Entity e = theEntityManager.CreateEntity("i");
                ADD_COMPONENT(e, Transformation);
                TRANSFORM(e)->position = game->grid.gridPosToPosition(i);
                TRANSFORM(e)->size = glm::vec2(1.f);
                TRANSFORM(e)->z = 0.9;

                ADD_COMPONENT(e, Rendering);
                RENDERING(e)->color = Color(1,0,1,1);
                RENDERING(e)->show = true;

                ADD_COMPONENT(e, AutoDestroy);
                AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;
                AUTO_DESTROY(e)->params.lifetime.freq.value = 3;
            }
            for (auto i: s) {
                Entity e = theEntityManager.CreateEntity("i");
                ADD_COMPONENT(e, Transformation);
                TRANSFORM(e)->position = game->grid.gridPosToPosition(i);
                TRANSFORM(e)->size = glm::vec2(1.f);
                TRANSFORM(e)->z = 0.99;

                ADD_COMPONENT(e, Rendering);
                RENDERING(e)->color = Color(0,1,0,1);
                RENDERING(e)->show = true;
                RENDERING(e)->shape = Shape::Hexagon;

                ADD_COMPONENT(e, AutoDestroy);
                AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;
                AUTO_DESTROY(e)->params.lifetime.freq.value = 3;
            }
        }

        return Scene::SelectCharacter;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
    bool updatePreExit(Scene::Enum, float) override {return true;}
    void onExit(Scene::Enum) override {

    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateSelectCharacterSceneHandler(PrototypeGame* game) {
        return new SelectCharacterScene(game);
    }
}
