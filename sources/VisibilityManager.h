#pragma once

#include "base/Entity.h"
#include "util/SpatialGrid.h"
#include <vector>
#include <map>

class VisibilityManager {
	public:
		void init(SpatialGrid& grid);
		void reset();
		void updateVisibility(const SpatialGrid& grid, const GridPos& viewerPos, int viewDistance);
		void toggleVisibility(bool show);

	private:
		bool show;
		std::vector<GridPos> visible;
		std::map<GridPos, Entity> entities;
};