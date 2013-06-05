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
#include "systems/TransformationSystem.h"
#include "systems/SoldierSystem.h"
#include "systems/ButtonSystem.h"
#include <glm/gtx/compatibility.hpp>

struct SelectActionScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    std::vector<Entity> moves;

    // Scene variables

    SelectActionScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~SelectActionScene() {}

    // Setup internal var, states, ...
    void setup() override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreEnter(Scene::Enum) {}
    bool updatePreEnter(Scene::Enum, float) override {return true;}
    void onEnter(Scene::Enum) override {
        // mark all moveable tile
        Entity active = game->activeCharacter;
        GridPos pos = game->grid.positionToGridPos(TRANSFORM(active)->position);

        const int maxRange = SOLDIER(active)->moveRange;
        std::map<int, std::vector<GridPos> > v = game->grid.movementRange(pos, maxRange);
        Color green1(0,1,0, 0.5);
        Color green2(0.2,0.4,0, 0.5);
        for (auto i: v) {
            if (i.first > 0) {
                for (auto gridPos: i.second) {
                    Entity e = theEntityManager.CreateEntity("gridcell",
                    EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
                    TRANSFORM(e)->position = game->grid.gridPosToPosition(gridPos);
                    float t = i.first / (float)maxRange;
                    RENDERING(e)->color = green2 * t + green1 * (1 - t);
                    RENDERING(e)->show = true;
                    moves.push_back(e);
                }
            }
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(game->activeCharacter)->clicked)
            return Scene::SelectCharacter;

        return Scene::SelectAction;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
    bool updatePreExit(Scene::Enum, float) override {return true;}
    void onExit(Scene::Enum) override {
        for (auto e: moves) {
            theEntityManager.DeleteEntity(e);
        }
        moves.clear();
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateSelectActionSceneHandler(PrototypeGame* game) {
        return new SelectActionScene(game);
    }
}
