#include "LevelSystem.h"
#include "base/Log.h"
#include "base/Color.h"
#include "base/PlacementHelper.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include <fstream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>

INSTANCE_IMPL(LevelSystem);

LevelSystem::LevelSystem() : ComponentSystemImpl <LevelComponent>("Level") {
    LevelComponent lc;
 //   componentSerializer.add(new Property<std::string>("ascii_map", OFFSET(asciiMap, lc)));
}

void LevelSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Level, e, lc)
        LOGI(theEntityManager.entityName(e) << ":" << lc->asciiMap);
    }
}



void LevelSystem::LoadFromFile(const std::string & filename) {
    std::ifstream myfile (filename);

    std::vector<Entity> blocks;

    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

    std::string line;

    while (std::getline(myfile, line)) {
        std::istringstream iss(line);

        glm::vec2 firstPoint, secondPoint;

        iss >> firstPoint.x;
        iss.ignore(1, ',');
        iss >> firstPoint.y;

        iss.ignore(1, '|');

        iss >> secondPoint.x;
        iss.ignore(1, ',');
        iss >> secondPoint.y;


        Entity e = theEntityManager.CreateEntity("block",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (secondPoint + firstPoint) / 2.f;
        TRANSFORM(e)->size.x = glm::length(secondPoint - firstPoint);
        TRANSFORM(e)->size.y = 0;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( secondPoint - firstPoint));

        LOGI(TRANSFORM(e)->position << " " << TRANSFORM(e)->size << " " << TRANSFORM(e)->rotation);
    }

    myfile.close();
}
