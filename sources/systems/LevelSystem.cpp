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

#if SAC_EMSCRIPTEN
    std::list<std::pair<glm::vec2, glm::vec2>> points;
#endif

void LevelSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Level, e, lc)
        LOGI(theEntityManager.entityName(e) << ":" << lc->asciiMap);
    }
}

void LevelSystem::SaveInFile(const std::string & filename, const std::list<Entity> & list) {
#if ! SAC_EMSCRIPTEN
    //save in file each walls
    std::ofstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");
#endif

    for (auto wall : list) {
        TransformationComponent * tc = TRANSFORM(wall);

        glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);
#if SAC_EMSCRIPTEN
        points.push_back(std::make_pair(tc->position - offset, tc->position + offset));
    }
#else
        myfile << tc->position - offset << " | " << tc->position + offset << "\n";
    }
    myfile.close();
#endif
}

void LevelSystem::LoadFromFile(const std::string & filename) {
#if ! SAC_EMSCRIPTEN
    std::ifstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");
#endif

    std::vector<Entity> blocks;


#if SAC_EMSCRIPTEN
    for (auto pair : points) {
        Entity e = theEntityManager.CreateEntity("block",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (pair.first + pair.second) / 2.f;
        TRANSFORM(e)->size.x = glm::length(pair.second - pair.first);
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( pair.second - pair.first));
    }
#else
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
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( secondPoint - firstPoint));
    }

    myfile.close();
#endif
}
