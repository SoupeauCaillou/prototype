#include "VisibilityManager.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

static Entity createVisibilityEntity() {
	Entity e = theEntityManager.CreateEntity("visibility",
        EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("cell"));
	TRANSFORM(e)->z = 0.95;
	TRANSFORM(e)->size *= 1.1;
	RENDERING(e)->color = Color(0.2, 0.2, 0.2, 1);
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

void VisibilityManager::reset() {
	for (auto pe: entities) {
		RENDERING(pe.second)->show = show;
	}
	visible.clear();
}

void VisibilityManager::updateVisibility(const SpatialGrid& grid, const GridPos& viewerPos, int viewDistance) {
	// lookup visible tiles
	std::vector<GridPos> vis = grid.viewRange(viewerPos, viewDistance);

	for (const GridPos& gp : vis) {
		if (std::find(visible.begin(), visible.end(), gp) == visible.end()) {
			RENDERING(entities[gp])->show = false;
			visible.push_back(gp);
		}
	}
}