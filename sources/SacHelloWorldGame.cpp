#include "SacHelloWorldGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/CameraSystem.h"

SacHelloWorldGame::SacHelloWorldGame() : grid(9,9,(glm::sqrt(3.0f) * 0.5f)) {
}

void SacHelloWorldGame::init(const uint8_t*, int) {
    grid.forEachCellDo([this] (const GridPos& pos) -> void {
        std::string type = std::string("field/cell_") +( rand()%2?"grass":"rock");

        Entity e = theEntityManager.CreateEntityFromTemplate(type.c_str());
        TRANSFORM(e)->position = grid.gridPosToPosition(pos);
        grid.addEntityAt(e, pos);
    });

     CAMERA(theEntityManager.getEntityByName(HASH("camera",0x526b9e0c)))->clearColor = Color(0,0,0);
}

bool SacHelloWorldGame::wantsAPI(ContextAPI::Enum api) const {
        switch (api) {
                case ContextAPI::Asset:
                        return true;
                default:
                        return false;
        }
}

void SacHelloWorldGame::tick(float) {

}
