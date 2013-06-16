#pragma once

#include "base/Entity.h"
#include "util/SpatialGrid.h"
#include <vector>
#include <map>

class VisibilityManager {
	public:
		void init(SpatialGrid& grid);

		void updateVisibility(const std::vector<Entity>& viewers);
		void toggleVisibility(bool show);

	private:
		bool show;
		std::map<GridPos, Entity> entities;
};
