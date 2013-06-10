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

struct SelectActionScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    std::vector<Entity> moves, attacks;

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
        const GridPos pos = game->grid.positionToGridPos(TRANSFORM(active)->position);

        const int pointsLeft = PLAYER(game->humanPlayer)->actionPointsLeft;
        std::map<int, std::vector<GridPos> > v = game->grid.movementRange(pos, pointsLeft);
        Color green1(0,1,0, 0.5);
        Color green2(0.2,0.4,0, 0.5);
        for (auto i: v) {
            if (i.first > 0 && i.first <= pointsLeft) {
                for (auto gridPos: i.second) {
                    Entity e = theEntityManager.CreateEntity("potential_move",
                    EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
                    TRANSFORM(e)->position = game->grid.gridPosToPosition(gridPos);
                    float t = i.first / (float)pointsLeft;
                    RENDERING(e)->color = green2 * t + green1 * (1 - t);
                    RENDERING(e)->show = true;
                    ADD_COMPONENT(e, Button);
                    BUTTON(e)->enabled = true;
                    BUTTON(e)->overSize = 0.8;
                    moves.push_back(e);
                }
            }
        }

        // mark attack possibilities
        if (pointsLeft >= 2) {
            unsigned atkRange = SOLDIER(game->activeCharacter)->attackRange;
            for (auto enemy: game->yEnnemies) {
                const GridPos enemyPos = game->grid.positionToGridPos(TRANSFORM(enemy)->position);
                if (SpatialGrid::ComputeDistance(pos, enemyPos) <= atkRange) {
                    if (game->grid.canDrawLine(pos, enemyPos)) {
                        Entity e = theEntityManager.CreateEntity("potential_atk",
                        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
                        TRANSFORM(e)->position = TRANSFORM(enemy)->position;
                        RENDERING(e)->color = Color(0.6, 0.1, 0.1);
                        RENDERING(e)->show = true;
                        attacks.push_back(e);
                    }
                }
            }
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        // Camera movement
        if (theCameraMoveManager.update(dt, game->camera))
            return Scene::SelectAction;
        if (BUTTON(game->banner)->clicked)
            return Scene::EndTurn;

        // Change active character
        for (auto p: game->players) {
            if (BUTTON(p)->clicked) {
                if (game->activeCharacter == p) {
                    return Scene::SelectCharacter;
                } else {
                    game->activeCharacter = p;
                    onExit(Scene::SelectCharacter);
                    onEnter(Scene::SelectCharacter);
                    return Scene::SelectAction;
                }
            }
        }

        // Move action ?
        for (auto e: moves) {
            if (BUTTON(e)->clicked) {
                const GridPos from =
                    game->grid.positionToGridPos(TRANSFORM(game->activeCharacter)->position);
                const GridPos to =
                    game->grid.positionToGridPos(TRANSFORM(e)->position);
                std::vector<GridPos> steps = game->grid.findPath(from, to);
                LOGF_IF(steps.empty(), "Could not find path: " << from << " -> " << to);

                // Create Move action
                Entity previousAction = 0;
                for (const auto& gp : steps) {
                    Entity action = theEntityManager.CreateEntity("move_action",
                    EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
                    RENDERING(action)->color = Color(0.8, 0.7, 0, 0.5);
                    RENDERING(action)->show = true;
                    TRANSFORM(action)->position = game->grid.gridPosToPosition(gp);
                    ADD_COMPONENT(action, Action);
                    ACTION(action)->type = Action::MoveTo;
                    ACTION(action)->entity = game->activeCharacter;
                    ACTION(action)->moveToTarget = game->grid.gridPosToPosition(gp);
                    LOGI("Move Target, " << ACTION(action)->moveToTarget);
                    ACTION(action)->moveSpeed = 3;
                    ACTION(action)->dependsOn = previousAction;
                    previousAction = action;
                }
                return Scene::ExecuteAction;
            }
        }

        if (PLAYER(game->humanPlayer)->actionPointsLeft >= 2) {
            // Attack action
            for (auto e: game->yEnnemies) {
                if (BUTTON(e)->clicked) {
                    const GridPos& myPos = game->grid.positionToGridPos(TRANSFORM(game->activeCharacter)->position);
                    const GridPos& enemyPos = game->grid.positionToGridPos(TRANSFORM(e)->position);

                    if (game->grid.canDrawLine(myPos, enemyPos)) {
                        Entity action = theEntityManager.CreateEntity("atk_action",
                        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
                        RENDERING(action)->color = Color(0.8, 0.2, 0.2, 0.5);
                        RENDERING(action)->show = true;
                        RENDERING(action)->shape = Shape::Triangle;
                        RENDERING(action)->dynamicVertices = 0;
                        ADD_COMPONENT(action, Action);
                        ACTION(action)->type = Action::Attack;
                        ACTION(action)->entity = game->activeCharacter;
                        ACTION(action)->attackTarget = e;
                        TRANSFORM(action)->position = glm::vec2(0.0f);
                        TRANSFORM(action)->size = glm::vec2(1.0f);

                        const auto p1 = TRANSFORM(game->activeCharacter)->position;
                        const auto p2 = TRANSFORM(e)->position;
                        const auto diff = glm::normalize(p2 - p1);
                        std::vector<glm::vec2> points = {
                            p1 + glm::vec2(diff.y, -diff.x) * TRANSFORM(game->activeCharacter)->size * 0.1f,
                            p1 + glm::vec2(diff.y, -diff.x) * TRANSFORM(game->activeCharacter)->size * -0.1f,
                            p2
                        };
                        theRenderingSystem.defineDynamicVertices(0, points);
                        return Scene::ExecuteAction;
                    }
                }
            }
        }

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
        for (auto a: attacks) {
            theEntityManager.DeleteEntity(a);
        }
        attacks.clear();
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateSelectActionSceneHandler(PrototypeGame* game) {
        return new SelectActionScene(game);
    }
}
