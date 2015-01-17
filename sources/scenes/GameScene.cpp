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

#include "systems/ADSRSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/MusicSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/GridSystem.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../SacHelloWorldGame.h"

#include "base/SceneState.h"


class GameScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;
    Entity dog;
    // Which GridPos are occupied for next turn, and by whom.
    std::vector<std::pair<GridPos, Entity>> unavailable;

    public:

    GameScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("game", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    void onPreEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreEnter(f);
    }

    void onEnter(Scene::Enum f) {
        SceneState<Scene::Enum>::onEnter(f);

        game->grid.forEachCellDo([this] (const GridPos& pos) -> void {
            std::string type = std::string("field/cell_grass");

            Entity e = theEntityManager.CreateEntityFromTemplate(type.c_str());
            std::ostringstream iss;
            iss << theEntityManager.entityName(e) << "(" << pos.q << "/" << pos.r << ")";
            theEntityManager.renameEntity(e, HASH(iss.str().c_str(), 0));
            game->grid.addEntityAt(e, pos, true);
        });
        dog = theEntityManager.CreateEntityFromTemplate("dog");
        game->grid.addEntityAt(dog, GridPos(0, 0), true);

        Entity sheep = theEntityManager.CreateEntityFromTemplate("sheep");
        game->grid.addEntityAt(sheep, GridPos(1, 0), true);
        sheep = theEntityManager.CreateEntityFromTemplate("sheep");
        game->grid.addEntityAt(sheep, GridPos(1, 1), true);
    }

    Scene::Enum update(float) {
        GridPos dogPos = game->grid.positionToGridPos(TRANSFORM(dog)->position);
        /*from dog pos, verify if any of its neighbor has been clicked*/
        for (auto & neighbor : game->grid.getNeighbors(dogPos)) {
            for (Entity elem : game->grid.getEntitiesAt(neighbor)) {
                if (theButtonSystem.Get(elem, false) && BUTTON(elem)->clicked) {
                    //if possible move the dog
                    unavailable.clear();
                    if (moveToPosition(dog, dogPos, neighbor)) {
                        for (auto pair : unavailable) {
                            game->grid.removeEntityFrom(
                                pair.second,
                                game->grid.positionToGridPos(TRANSFORM(pair.second)->position));
                            game->grid.addEntityAt(pair.second, pair.first, true);
                        }
                        return Scene::Game;
                    }
                }
            }
        }
        return Scene::Game;
    }

    void onPreExit(Scene::Enum f) {
        SceneState<Scene::Enum>::onPreExit(f);
    }

    void onExit(Scene::Enum to) {
        SceneState<Scene::Enum>::onExit(to);
    }

    bool isGameElement(Entity e, bitfield_t game_type) {
        return theGridSystem.Get(e, false) && (GRID(e)->type&game_type) != 0;
    }

    GridPos findDirection(GridPos& incoming, GridPos& current) {
        std::vector<GridPos> neighbors = game->grid.getNeighbors(current);
        std::sort(neighbors.begin(), neighbors.end(), [incoming] (GridPos& a, GridPos& b) -> bool {
            return true;
        });
        for (auto & pos : neighbors) {
            auto find = std::find_if(unavailable.begin(), unavailable.end(), [pos] (std::pair<GridPos, Entity> p) {
                return pos == p.first;
            });
            if (find == unavailable.end()) {
                return pos;
            }
        }
        //no position available
        return GridPos(-1,-1);
    }

    /*
     * 1) Dog will move to the given position.
     *  a) If the cell is empty, nothing happen.
     *  b) If a sheep is at this position, it must move. It will consider any position
     *      around him (except dogs', rocks' ones), ignoring any other sheep
     *      I) if it can go straight, it will
     *      II) otherwise if it can go left-straight or right-straight cells, it will
     *      III) otherwise if it can go left or right cells, it will
     *  c) If another sheep is on the given position, it has to move: go to 2)
     *  d) Finally, any sheep in the neighborhood of a moving sheep will try to move
     *      on the same direction, if it can. If the cell is unavailable and/or
     *      already occupied by another sheep, it will not move.
     */
    bool moveToPosition(Entity inc, GridPos& from, GridPos& to) {
        unavailable.push_back(std::make_pair(to, inc));
        for (Entity e : game->grid.getEntitiesAt(to)) {
            if (isGameElement(e, Case::Dog | Case::Sheep)) {
                // 2) sheep in place must be moved
                GridPos dir = findDirection(from, to);

                if (dir != GridPos(-1, -1)) {
                    return moveToPosition(e, to, dir);
                } else {
                    LOGE("Could move from position " << to);
                    return false;
                }
            }
        }
        // 1) cell is empty
        return true;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameSceneHandler(SacHelloWorldGame* game) {
        return new GameScene(game);
    }
}
