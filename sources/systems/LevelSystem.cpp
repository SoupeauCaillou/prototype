#include "LevelSystem.h"
#include "base/Log.h"
#include "base/Color.h"
#include "base/PlacementHelper.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/BlockSystem.h"

#include <fstream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>

INSTANCE_IMPL(LevelSystem);

LevelSystem::LevelSystem() : ComponentSystemImpl <LevelComponent>("Level") {
    LevelComponent lc;
}

void LevelSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Level, e, lc)
        LOGI(theEntityManager.entityName(e) << ":" << lc->asciiMap);
    }
}

#if SAC_EMSCRIPTEN
std::list<std::pair<std::pair<glm::vec2, glm::vec2>, bool>> points;
void LevelSystem::SaveInFile(const std::string &, const std::list<Entity> & list) {
    for (auto wall : list) {
        TransformationComponent * tc = TRANSFORM(wall);

        glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);
        points.push_back(std::make_pair(std::make_pair(tc->position - offset, tc->position + offset), false));
    }
}

#else
void LevelSystem::SaveInFile(const std::string & filename, const std::list<Entity> & list) {
    //save in file each walls
    std::ofstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

    //spot number
    myfile << "1\n";

    for (auto wall : list) {
        TransformationComponent * tc = TRANSFORM(wall);

        glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);

        myfile << tc->position - offset << " | " << tc->position + offset << " false\n";
    }
    myfile.close();
}
#endif

void LevelSystem::LoadFromFile(const std::string & filename) {
#if SAC_EMSCRIPTEN
    Entity e = theEntityManager.CreateEntity("spot",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("spot"));

    for (auto pair : points) {
        Entity e = theEntityManager.CreateEntity("block",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (pair.first.first + pair.first.second) / 2.f;
        TRANSFORM(e)->size.x = glm::length(pair.first.second - pair.first.first);
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( pair.first.second - pair.first.first));
        BLOCK(e)->doubleFace = pair.second;
    }
#else
    std::ifstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");
    std::string line;

    std::getline (myfile, line);
    std::istringstream iss(line);
    int spotCount = 0;
    iss >> spotCount;
    while (--spotCount >= 0) {
        Entity e = theEntityManager.CreateEntity("spot",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("spot"));
        TRANSFORM(e)->position.x = TRANSFORM(e)->position.y = 4*spotCount;
    }

    while (std::getline(myfile, line)) {
        std::istringstream iss2(line);

        glm::vec2 firstPoint, secondPoint;
        std::string doubleFace;

        iss2 >> firstPoint.x;
        iss2.ignore(1, ',');
        iss2 >> firstPoint.y;

        iss2.ignore(1, '|');

        iss2 >> secondPoint.x;
        iss2.ignore(1, ',');
        iss2 >> secondPoint.y;

        iss2 >> doubleFace;

        Entity e = theEntityManager.CreateEntity("block",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (secondPoint + firstPoint) / 2.f;
        TRANSFORM(e)->size.x = glm::length(secondPoint - firstPoint);
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( secondPoint - firstPoint));
        BLOCK(e)->doubleFace = (doubleFace == "true");
    }

    myfile.close();
#endif
}
