#include "SacHelloWorldGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/CameraSystem.h"

SacHelloWorldGame::SacHelloWorldGame() : grid(9,9,(glm::sqrt(3.0f) * 0.5f)) {
}

void SacHelloWorldGame::init(const uint8_t*, int) {
    grid.forEachCellDo([this] (const GridPos& pos) -> void {
        std::string type = std::string("field/cell_") +( rand()%2?"grass":"rock");

        Entity e = theEntityManager.CreateEntityFromTemplate(type.c_str());
        grid.addEntityAt(e, pos, true);
    });

    CAMERA(theEntityManager.getEntityByName(HASH("camera",0x526b9e0c)))->clearColor = Color(0,0,0);

    dog = theEntityManager.CreateEntityFromTemplate("dog");
    grid.addEntityAt(dog, GridPos(0, 0), true);

    Entity sheep = theEntityManager.CreateEntityFromTemplate("sheep");
    grid.addEntityAt(sheep, GridPos(1, 0), true);
    sheep = theEntityManager.CreateEntityFromTemplate("sheep");
    grid.addEntityAt(sheep, GridPos(1, 1), true);
}

bool SacHelloWorldGame::wantsAPI(ContextAPI::Enum api) const {
        switch (api) {
                case ContextAPI::Asset:
                        return true;
                default:
                        return false;
        }
}

void SacHelloWorldGame::moveToPosition(Entity dog, GridPos& pos) {

    // finally move the dog to its position



    grid.addEntityAt(dog, pos, true);
}

void SacHelloWorldGame::tick(float) {
    GridPos dogPos = grid.positionToGridPos(TRANSFORM(dog)->position);
    /*from dog pos, very if any of its neighbor has been clicked*/
    for (auto & neighbor : grid.getNeighbors(dogPos)) {
        for (Entity elem : grid.getEntitiesAt(neighbor)) {
            if (theButtonSystem.Get(elem, false) && BUTTON(elem)->clicked) {
                //if so, move the dog and return
                grid.removeEntityFrom(dog, dogPos);
                moveToPosition(dog, neighbor);
                return;
            }
        }
    }
}
