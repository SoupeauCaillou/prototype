#include "SacHelloWorldGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/GridSystem.h"
#include "systems/CameraSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/GridSystem.h"

SacHelloWorldGame::SacHelloWorldGame() : grid(11,9,2.6) {
}

void SacHelloWorldGame::init(const uint8_t*, int) {
    grid.forEachCellDo([this] (const GridPos& pos) -> void {
        std::string type = std::string("field/cell_grass");

        Entity e = theEntityManager.CreateEntityFromTemplate(type.c_str());
        std::ostringstream iss;
        iss << theEntityManager.entityName(e) << "(" << pos.q << "/" << pos.r << ")";
        theEntityManager.renameEntity(e, HASH(iss.str().c_str(), 0));
        grid.addEntityAt(e, pos, true);
    });

    CAMERA(theEntityManager.getEntityByName(HASH("camera",0x526b9e0c)))->clearœColor = Color(0,0,0);

    dog = theEntityManager.CreateEntityFromTemplate("dog");
    grid.addEntityAt(dog, GridPos(0, 0), true);

    Entity sheep = theEntityManager.CreateEntityFromTemplate("sheep");
    grid.addEntityAt(sheep, GridPos(1, 0), true);
    sheep = theEntityManager.CreateEntityFromTemplate("sheep");
    grid.addEntityAt(sheep, GridPos(1, 1), true);
    TRANSFORM(camera)->size = glm::vec2(28, 17);

    registerScenes(this, sceneStateMachine);
    sceneStateMachine.setup(gameThreadContext->assetAPI);
    sceneStateMachine.start(Scene::Editor);
}

bool SacHelloWorldGame::wantsAPI(ContextAPI::Enum api) const {
        switch (api) {
                case ContextAPI::Asset:
                        return true;
                default:
                        return false;
        }
}

static bool isGameElement(Entity e, bitfield_t game_type) {
    return theGridSystem.Get(e, false) && (GRID(e)->type & game_type) != 0;
}

GridPos SacHelloWorldGame::findDirection(GridPos& incoming, GridPos& current) {
    std::vector<GridPos> neighbors = grid.getNeighbors(current);
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

/**
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
bool SacHelloWorldGame::moveToPosition(Entity inc, GridPos& from, GridPos& to) {
    unavailable.push_back(std::make_pair(to, inc));
    for (Entity e : grid.getEntitiesAt(to)) {
        if (isGameElement(e, SacHelloWorldGame::GameElement::Dog | SacHelloWorldGame::GameElement::Sheep)) {
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


void SacHelloWorldGame::tick(float dt) {
    GridPos dogPos = grid.positionToGridPos(TRANSFORM(dog)->position);
    /*from dog pos, verify if any of its neighbor has been clicked*/
    for (auto & neighbor : grid.getNeighbors(dogPos)) {
        for (Entity elem : grid.getEntitiesAt(neighbor)) {
            if (theButtonSystem.Get(elem, false) && BUTTON(elem)->clicked) {
                //if possible move the dog
                unavailable.clear();
                if (moveToPosition(dog, dogPos, neighbor)) {
                    for (auto pair : unavailable) {
                        grid.removeEntityFrom(
                            pair.second,
                            grid.positionToGridPos(TRANSFORM(pair.second)->position));
                        grid.addEntityAt(pair.second, pair.first, true);
                    }
                }
                return;
            }
        }
    }
    sceneStateMachine.update(dt);
}
