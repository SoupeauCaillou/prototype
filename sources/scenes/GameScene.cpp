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
#include "Scenes.h"
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
    std::vector<std::pair<GridPos, Entity>> unavailable;

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
                    unavailable.push_back(std::make_pair(pos, 0));
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

    void updateMovingSheepList(GridPos from, GridPos to, bool moveNeighbourSheeps = true) {
        // if there's a sheep @to -> it must move
        {
            Entity s = gameElementAt(to, Case::Sheep);
            if (s) {
                if (!didSheepMove(s)) {
                    mandatoryMovingSheeps.push_back(std::make_pair(s, to - from));
                    staticSheeps.remove(s);
                } else {
                    LOGE("Cant move...");
                }
            }
        }
        // if there are neighbour sheep @from -> they should move
        if (moveNeighbourSheeps) {
            for (auto & neighbor : game->grid->getNeighbors(from)) {
                Entity s = gameElementAt(neighbor, Case::Sheep);
                if (s) {
                    if (!didSheepMove(s)) {
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
                            unavailable.push_back(std::make_pair(neighbor, dog));
                            // move dog to neighbor
                            //....
                            dogHasMoved = true;

                            updateMovingSheepList(dogPos, neighbor, false);
                            LOGI(theEntityManager.entityName(dog) << " moved to " << neighbor);
                            Entity e = theEntityManager.CreateEntityFromTemplate("move_command");
                            MOVE_CMD(e)->target = dog;
                            MOVE_CMD(e)->from = dogPos;
                            MOVE_CMD(e)->to = neighbor;
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
            GridPos chosen = findDirection(position - dir, position);
            if (chosen == invalidGridPos) {
                LOGE("Cannot find a valid move for sheep " << theEntityManager.entityName(sheep));
            }
            unavailable.push_back(std::make_pair(chosen, sheep));
            mandatoryMovingSheeps.pop_front();

            updateMovingSheepList(position, chosen);


            LOGI(theEntityManager.entityName(sheep) << " moved to " << chosen);
            // move sheep
            Entity e = theEntityManager.CreateEntityFromTemplate("move_command");
            MOVE_CMD(e)->target = sheep;
            MOVE_CMD(e)->from = position;
            MOVE_CMD(e)->to = chosen;

            return Scene::Game;
        }

        if (!optionallyMovingSheeps.empty()) {
            auto& sheepDir = optionallyMovingSheeps.front();
            Entity sheep = sheepDir.first;
            GridDirection dir = sheepDir.second;
            GridPos position = game->grid->positionToGridPos(TRANSFORM(sheep)->position);
            // prefered new position
            GridPos chosen = position + dir;
            if (!cellIsAvailable(chosen)) {
                LOGI("Cannot find a valid move for sheep " << theEntityManager.entityName(sheep));
            } else {
                unavailable.push_back(std::make_pair(chosen, sheep));
                updateMovingSheepList(position, chosen);
                // move sheep
                // ...
                LOGI(theEntityManager.entityName(sheep) << " moved to " << chosen);
                Entity e = theEntityManager.CreateEntityFromTemplate("move_command");
                MOVE_CMD(e)->target = sheep;
                MOVE_CMD(e)->from = position;
                MOVE_CMD(e)->to = chosen;
            }
            optionallyMovingSheeps.pop_front();


            return Scene::Game;
        }
        LOGI("exit");

        game->updateMovesCount(game->movesCount+1);
        return Scene::Moving;
#if 0

                     && moveToPosition(dog, dogPos, neighbor)) {
                        for (auto pair : unavailable) {
                            if (pair.second != 0) {
                                game->grid->removeEntityFrom(
                                    pair.second,
                                    game->grid->positionToGridPos(TRANSFORM(pair.second)->position));
                                game->grid->addEntityAt(pair.second, pair.first, true);
                            }
                        }
                        return Scene::Game;
                    }
                }
            }
        }
        return Scene::Game;
#endif
    }

    void onPreExit(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreExit(f);
    }

    void onExit(Scene::Enum to) override {
        SceneState<Scene::Enum>::onExit(to);
    }

    bool cellIsAvailable(GridPos pos) {
        auto find = std::find_if(unavailable.begin(), unavailable.end(), [pos] (std::pair<GridPos, Entity> p) {
            return pos == p.first;
        });
        return find == unavailable.end();
    }

    bool isGameElement(Entity e, bitfield_t gameType) {
        bitfield_t type = GRID(e)->type;
        return theGridSystem.Get(e, false) && (type & gameType) != 0;
    }

    GridPos findDirection(const GridPos& incoming, const GridPos& current) {
        std::vector<GridPos> neighbors = game->grid->getNeighbors(current);
        std::sort(neighbors.begin(), neighbors.end(), [this, incoming] (GridPos& a, GridPos& b) -> bool {
            return game->grid->computeRealDistance(incoming, a) > game->grid->computeRealDistance(incoming, b) ;
        });
        for (auto & pos : neighbors) {
           if (cellIsAvailable(pos)) {
                return pos;
            }
        }
        //no position available
        return invalidGridPos;
    }

    /*
     * 1) Dog or sheep will move to the given position.
     *  a) If the cell is empty, nothing happen.
     *  b) If a sheep is at this position, it must move. It will consider any position
     *      around him (except dogs', rocks' ones), ignoring any other sheep
     *      I) if it can go straight, it will
     *      II) otherwise if it can go left-straight or right-straight cells, it will
     *      III) otherwise if it can go left or right cells, it will
     *  c) If another sheep is on the given position, it has to move: go to 2)
     * 2) Finally, any sheep in the neighborhood of a moving sheep will try to move
     *     on the same direction, if it can. If the cell is unavailable and/or
     *     already occupied by another sheep, it will not move.
     */
    bool moveToPosition(Entity inc, GridPos& from, GridPos& to) {
        unavailable.push_back(std::make_pair(to, inc));
        for (Entity e : game->grid->getEntitiesAt(to)) {
            if (isGameElement(e, Case::Dog | Case::Sheep)) {
                GridPos dir = findDirection(from, to);

                if (dir != invalidGridPos) {
                    return moveToPosition(e, to, dir);
                } else {
                    LOGE("Could not move from position " << to);
                    return false;
                }
            }
        }
        return true;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameSceneHandler(SacHelloWorldGame* game) {
        return new GameScene(game);
    }
}
