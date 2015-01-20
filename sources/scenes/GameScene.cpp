/*
    This file is part of RecursiveRunner.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

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
#include "gen/Scenes.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"

#include "systems/ADSRSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/MusicSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/GridSystem.h"

#include "util/Draw.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../SacHelloWorldGame.h"

#include "../Level.h"
#include "base/SceneState.h"
#include "systems/MoveCommandSystem.h"

class GameScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;
    Entity dog;
    // Which GridPos are occupied for next turn, and by whom.
    std::vector<GridPos> unavailable;

    typedef GridPos GridDirection;
    bool dogHasMoved;
    std::list<Entity> staticSheeps;
    std::list<std::pair<Entity, GridDirection>> mandatoryMovingSheeps;
    std::list<std::pair<Entity, GridDirection>> optionallyMovingSheeps;
    std::vector<Entity> moveCommands;

    public:

    GameScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("game", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    void onPreEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreEnter(f);
    }

    void onEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onEnter(f);

        dog = 0;
        theGridSystem.forEachECDo([this] (Entity e, GridComponent* gc) -> void {
            if (gc->type == Case::Dog) {
                LOGI("dog found: " << e);
                dog = e;
            }
        });
        LOGF_IF(!dog, "No dog defined");

        unavailable.clear();
        staticSheeps.clear();
        mandatoryMovingSheeps.clear();
        optionallyMovingSheeps.clear();
        dogHasMoved = false;

        /* look up initial unavailable tiles */
        game->grid->forEachCellDo([this] (const GridPos& pos) -> void {
            for (Entity e : game->grid->getEntitiesAt(pos)) {
                if (isGameElement(e, Case::Rock)) {
                    unavailable.push_back(pos);
                    break;
                } else if (isGameElement(e, Case::Sheep)) {
                    staticSheeps.push_back(e);
                    break;
                }
            }
        });
    }

    Entity gameElementAt(GridPos pos, Case::Enum elt) {
        for (Entity e : game->grid->getEntitiesAt(pos)) {
            if (isGameElement(e, elt)) {
                return e;
            }
        }
        return 0;
    }

    bool didSheepMove(Entity e) {
        return std::find(staticSheeps.begin(), staticSheeps.end(), e) == staticSheeps.end();
    }

    std::vector<Entity> moves;

    void updateMovingSheepList(GridPos from, GridPos to, bool moveNeighbourSheeps = true) {
        LOGI("Update sheeps to move");
        // if there's a sheep @to -> it must move
        {
            Entity s = gameElementAt(to, Case::Sheep);
            if (s) {
                if (!didSheepMove(s)) {
                    LOGI("Add " << s << " to mandatory_move list");
                    mandatoryMovingSheeps.push_back(std::make_pair(s, to - from));
                    staticSheeps.remove(s);
                } else {
                    auto it = std::find_if(optionallyMovingSheeps.begin(), optionallyMovingSheeps.end(), [s] (const std::pair<Entity, GridDirection>& p) -> bool {
                    return p.first == s; });

                    if (it != optionallyMovingSheeps.end()) {
                        LOGI("Shift " << s << " to mandatory_move list");
                        mandatoryMovingSheeps.push_back(std::make_pair(s, to - from));
                        optionallyMovingSheeps.erase(it);
                    } else {
                        LOGE("Cant move..." << s);
                        for (auto e: optionallyMovingSheeps)
                            LOGI(e.first);
                    }
                }
            }
        }
        // if there are neighbour sheep @from -> they should move
        if (moveNeighbourSheeps) {
            for (auto & neighbor : game->grid->getNeighbors(from)) {
                Entity s = gameElementAt(neighbor, Case::Sheep);
                if (s) {
                    if (!didSheepMove(s)) {
                        LOGI("Add " << s << " to optional_move list");
                        optionallyMovingSheeps.push_back(std::make_pair(s, to - from));
                        staticSheeps.remove(s);
                    }
                }
            }
        }
    }

    Scene::Enum update(float) override {
        if (!dogHasMoved) {
            GridPos dogPos = game->grid->positionToGridPos(TRANSFORM(dog)->position);
            /*from dog pos, verify if any of its neighbor has been clicked*/
            for (auto & neighbor : game->grid->getNeighbors(dogPos)) {
                for (Entity elem : game->grid->getEntitiesAt(neighbor)) {
                    if (!dogHasMoved && theButtonSystem.Get(elem, false) && BUTTON(elem)->clicked) {
                        //if possible move the dog
                        if (cellIsAvailable(neighbor)) {
                            unavailable.push_back(neighbor);
                            // move dog to neighbor
                            //....
                            dogHasMoved = true;

                            LOGI("Dog moved");
                            updateMovingSheepList(dogPos, neighbor, false);
                            LOGI(theEntityManager.entityName(dog) << " moved to " << neighbor);
                            Entity e = theEntityManager.CreateEntityFromTemplate("move_command");
                            MOVE_CMD(e)->target = dog;
                            MOVE_CMD(e)->from = dogPos;
                            MOVE_CMD(e)->to = neighbor;
                            moves.push_back(e);
                            break;
                        }
                    }
                }
            }
            return Scene::Game;
        }
        if (!mandatoryMovingSheeps.empty()) {
            auto& sheepDir = mandatoryMovingSheeps.front();
            Entity sheep = sheepDir.first;
            GridDirection dir = sheepDir.second;
            GridPos position = game->grid->positionToGridPos(TRANSFORM(sheep)->position);
            // prefered new position
            // GridPos pref = position + dir;
            // find nearest available cell
            GridPos chosen = findDirection(position - dir, position, false);
            if (chosen == invalidGridPos) {
                chosen = findDirection(position - dir, position, true);
            }
            if (chosen == invalidGridPos) {
                LOGE("Cannot find a valid mandatory move for sheep " << sheep);
                /* invalid move -> cancel all other moves */
                for (auto e: moves) {
                    theEntityManager.DeleteEntity(e);
                }
                return Scene::Moving;
            }
            unavailable.push_back(chosen);
            mandatoryMovingSheeps.pop_front();
            LOGI("Sheep " << sheep << " moved.");

            updateMovingSheepList(position, chosen);


            LOGI(theEntityManager.entityName(sheep) << " moved to " << chosen);
            // move sheep
            Entity e = theEntityManager.CreateEntityFromTemplate("move_command");
            MOVE_CMD(e)->target = sheep;
            MOVE_CMD(e)->from = position;
            MOVE_CMD(e)->to = chosen;
            moves.push_back(e);

            return Scene::Game;
        }

        if (!optionallyMovingSheeps.empty()) {
            auto& sheepDir = optionallyMovingSheeps.front();
            Entity sheep = sheepDir.first;
            GridDirection dir = sheepDir.second;
            GridPos position = game->grid->positionToGridPos(TRANSFORM(sheep)->position);
            // prefered new position
            GridPos chosen = position + dir;
            if (!game->grid->isPosValid(chosen)) {
                LOGF("Fix me: all level should have a proper border");
            }
            {/*if (!cellIsAvailable(chosen)||!game->grid->isPosValid(chosen)) {
                LOGI("Cannot find a valid optional move for sheep " << sheep);
            } else {*/
                unavailable.push_back(chosen);
                LOGI("Sheep " << sheep << " moved.");
                updateMovingSheepList(position, chosen);
                // move sheep
                // ...
                LOGI(theEntityManager.entityName(sheep) << " moved to " << chosen);
                Entity e = theEntityManager.CreateEntityFromTemplate("move_command");
                MOVE_CMD(e)->target = sheep;
                MOVE_CMD(e)->from = position;
                MOVE_CMD(e)->to = chosen;
                moves.push_back(e);
            }
            optionallyMovingSheeps.pop_front();


            return Scene::Game;
        }

        return Scene::Moving;
    }

    void onPreExit(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreExit(f);
        game->updateMovesCount(game->movesCount+1);
        moves.clear();
    }

    void onExit(Scene::Enum to) override {
        SceneState<Scene::Enum>::onExit(to);
    }

    bool cellIsAvailable(GridPos pos, bool obstacleAllowed = false) {
        auto find = std::find_if(unavailable.begin(), unavailable.end(), [pos] (const GridPos& p) {
            return pos == p;
        });
        if (find == unavailable.end())
            return true;

        if (!obstacleAllowed) {
            return false;
        } else {
            GridPos p = *find;
            /* Rock are allowed */
            return (gameElementAt(p, Case::Rock) != 0);
        }
    }

    bool isGameElement(Entity e, bitfield_t gameType) {
        bitfield_t type = GRID(e)->type;
        return theGridSystem.Get(e, false) && (type & gameType) != 0;
    }

    GridPos findDirection(const GridPos& incoming, const GridPos& current, bool obstaclesAllowed) {
        std::vector<GridPos> neighbors = game->grid->getNeighbors(current);
        std::sort(neighbors.begin(), neighbors.end(), [this, incoming] (GridPos& a, GridPos& b) -> bool {
            return game->grid->computeRealDistance(incoming, a) > game->grid->computeRealDistance(incoming, b) ;
        });
        for (auto & pos : neighbors) {
           if (cellIsAvailable(pos, obstaclesAllowed)) {
                return pos;
            }
        }
        //no position available
        return invalidGridPos;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameSceneHandler(SacHelloWorldGame* game) {
        return new GameScene(game);
    }
}
