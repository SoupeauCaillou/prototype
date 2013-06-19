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
#include "MemorySystem.h"
#include "PrototypeGame.h"
#include "systems/SoldierSystem.h"
#include "systems/VisionSystem.h"
#include "systems/PlayerSystem.h"

INSTANCE_IMPL(MemorySystem);

MemorySystem::MemorySystem() : ComponentSystemImpl<MemoryComponent>("Memory") {
	MemoryComponent sc;

}

void MemorySystem::DoUpdate(float) {
	for (auto p: components) {
		const Entity e = p.first;
		const int turn = PLAYER(e)->turn;
		auto* memory = p.second;

        // browse my soldiers
        theSoldierSystem.forEachECDo([this, e, memory, turn] (Entity entity, SoldierComponent* sc) -> void {
            if (sc->player != e)
                return;
            auto* vision = VISION(entity);
            if (!vision->enabled)
                return;
            LOGI(theEntityManager.entityName(e) << " sees " << vision->visiblePositions.size() << " positions");
    		for (auto& gp: vision->visiblePositions) {
    			for (auto visibleEntity: game->grid.getEntitiesAt(gp)) {
    				// look up soldiers in visible entity
    				auto* soldier = theSoldierSystem.Get(visibleEntity, false);

    				if (soldier && soldier->player == memory->enemyOwner) {
                                            LOGI_IF(soldier,
                        theEntityManager.entityName(e) << " sees " << theEntityManager.entityName(visibleEntity) << " soldier");
    					memory->enemyLastKnownPos[visibleEntity] = std::make_pair(turn, gp);
    				}
    			}
    		}
        });
	}
}
