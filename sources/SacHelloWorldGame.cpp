#include "SacHelloWorldGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

SacHelloWorldGame::SacHelloWorldGame() : grid(9,9,1) {
}

void SacHelloWorldGame::init(const uint8_t*, int) {
    grid.forEachCellDo([this] (const GridPos& pos) -> void {
        Entity e = theEntityManager.CreateEntityFromTemplate("field/cell");
        TRANSFORM(e)->position = grid.gridPosToPosition(pos);
        grid.addEntityAt(e, pos);
    });
}

void SacHelloWorldGame::tick(float) {

}
