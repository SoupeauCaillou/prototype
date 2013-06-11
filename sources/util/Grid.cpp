#include "Grid.h"

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"

Grid & Grid::Instance() {
    static Grid _instance;

    return _instance;
}

void Grid::CreateGrid() {
    //create a grid
    int i = 0;
    for (; i < PlacementHelper::ScreenHeight + 1; ++i) {
        Entity e = theEntityManager.CreateEntity("grid_line",
          EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_line"));
        TRANSFORM(e)->position.y = -.5 + PlacementHelper::ScreenHeight / 2. - i;
        TRANSFORM(e)->size.y = 1.;
        Instance()._gridEntities.push_back(e);
    }

    for (int j = - 10; j <= 10; j += 2) {
        std::stringstream ss;
        ss << j;

        Entity e = theEntityManager.CreateEntity("grid_number_x " + ss.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
        TEXT_RENDERING(e)->text = ss.str();
        TRANSFORM(e)->position.x = j;
        Instance()._gridTextEntities.push_back(e);

        e = theEntityManager.CreateEntity("grid_number_y " + ss.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
        TEXT_RENDERING(e)->text = ss.str();
        TRANSFORM(e)->position.y = j;
        Instance()._gridTextEntities.push_back(e);
    }
}

void Grid::EnableGrid() {
    for (auto e : Instance()._gridTextEntities) {
        TEXT_RENDERING(e)->show = true;
    }
    for (auto e : Instance()._gridEntities) {
        RENDERING(e)->show = true;
    }
}

void Grid::DisableGrid() {
    for (auto e : Instance()._gridTextEntities) {
        TEXT_RENDERING(e)->show = false;
    }
    for (auto e : Instance()._gridEntities) {
        RENDERING(e)->show = false;
    }
}


