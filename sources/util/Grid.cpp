/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

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

#include "Grid.h"

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"

#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"

Grid & Grid::Instance() {
    static Grid _instance;

    return _instance;
}

void Grid::CreateGrid() {
    //create a grid
    int i = 0;
    for (; i < PlacementHelper::ScreenSize.y + 1; ++i) {
        Entity e = theEntityManager.CreateEntity("grid_line",
          EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_line"));
        TRANSFORM(e)->position.y = -.5 + PlacementHelper::ScreenSize.y / 2. - i;
        TRANSFORM(e)->size.y = 1.;
        Instance()._gridEntities.push_back(e);
    }

    for (int j = - 10; j <= 10; j += 2) {
        std::stringstream ss;
        ss << j;

        Entity e = theEntityManager.CreateEntity("grid_number_x " + ss.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("text"));
        TEXT(e)->text = ss.str();
        TRANSFORM(e)->position.x = j;
        Instance()._gridTextEntities.push_back(e);

        e = theEntityManager.CreateEntity("grid_number_y " + ss.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("text"));
        TEXT(e)->text = ss.str();
        TRANSFORM(e)->position.y = j;
        Instance()._gridTextEntities.push_back(e);
    }
}

void Grid::EnableGrid() {
    for (auto e : Instance()._gridTextEntities) {
        TEXT(e)->show = true;
    }
    for (auto e : Instance()._gridEntities) {
        RENDERING(e)->show = true;
    }
}

void Grid::DisableGrid() {
    for (auto e : Instance()._gridTextEntities) {
        TEXT(e)->show = false;
    }
    for (auto e : Instance()._gridEntities) {
        RENDERING(e)->show = false;
    }
}


