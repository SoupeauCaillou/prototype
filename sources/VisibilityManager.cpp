#include "VisibilityManager.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/VisionSystem.h"

static Entity createVisibilityEntity() {
	Entity e = theEntityManager.CreateEntity("visibility",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
	TRANSFORM(e)->z = 0.95;
	TRANSFORM(e)->size *= 1.1;
	RENDERING(e)->color.a = 1;// *= Color(0.2, 0.2, 0.2, 1);
	return e;
}

void VisibilityManager::init(SpatialGrid& grid) {
    grid.doForEachCell([this, grid] (const GridPos& gp) -> void {
    	Entity e = createVisibilityEntity();
    	TRANSFORM(e)->position = grid.gridPosToPosition(gp);
		RENDERING(e)->show = show;
		entities[gp] = e;
    });
}

void VisibilityManager::toggleVisibility(bool pShow) {
	if (pShow != show) {
		show = pShow;
		for (auto p: entities) {
			RENDERING(p.second)->show = show;
		}
	}
}

void VisibilityManager::updateVisibility(const std::vector<Entity>& viewers) {
    LOGT_EVERY_N(100, "Use Camera to browse only the visibile tiles");
    // ouch
    for (auto p : entities) {
        RENDERING(p.second)->show = true;
    }
    for (auto viewer: viewers) {
        const auto* vc = VISION(viewer);
        const auto itBegin = vc->visiblePositions.begin();
        const auto itEnd = vc->visiblePositions.end();

        for (auto p : entities) {
            if (RENDERING(p.second)->show) {
                if (std::find(itBegin, itEnd, p.first) != itEnd) {
                    RENDERING(p.second)->show = false;
                }
            }
        }
	}
}
