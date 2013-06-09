#include "LevelSystem.h"
#include "base/Log.h"
#include "base/Color.h"
#include "base/PlacementHelper.h"

#include "util/DrawSomething.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/BlockSystem.h"
#include "systems/SpotSystem.h"

#include <fstream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>

INSTANCE_IMPL(LevelSystem);

std::string LevelSystem::currentLevelPath = "../../assetspc/level1.map";

LevelSystem::LevelSystem() : ComponentSystemImpl <LevelComponent>("Level") {
    LevelComponent lc;
}

void LevelSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Level, e, lc)
        LOGI(theEntityManager.entityName(e) << ":" << lc->asciiMap);
    }
}

#if SAC_EMSCRIPTEN
std::vector<std::pair<std::pair<glm::vec2, glm::vec2>, bool>> walls;
std::vector<glm::vec2> spots;
void LevelSystem::SaveInFile(const std::string &, const std::list<Entity> & wallList, const std::list<Entity> & spotList) {
    for (auto spot : spotList) {
        spots.push_back(TRANSFORM(spot)->position);
    }

    for (auto wall : wallList) {
        TransformationComponent * tc = TRANSFORM(wall);

        glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);
        walls.push_back(std::make_pair(std::make_pair(tc->position - offset, tc->position + offset), false));
    }
}

#else
void LevelSystem::SaveInFile(const std::string & filename, const std::list<Entity> & wallList, const std::list<Entity> & spotList) {
    //save in file each walls
    std::ofstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

    //spot number
    myfile << spotList.size() << "\n";
    for (auto spot : spotList) {
        myfile << TRANSFORM(spot)->position << "\n";
    }

    for (auto wall : wallList) {
        auto * tc = TRANSFORM(wall);

        auto offset = glm::rotate(tc->size / 2.f, tc->rotation);

        myfile << tc->position - offset << " | " << tc->position + offset << " false\n";
    }
    myfile.close();
}
#endif

void LevelSystem::LoadFromFile(void * filename) {
    LoadFromFile(*((std::string *)filename));
}

void LevelSystem::LoadFromFile(const std::string & filename) {
    LOGI("Loading map '" << ((filename.size() != 0) ? filename : currentLevelPath) << "'...");

    //remove any existing things first
    Draw::DrawPointRestart("SpotSystem");
    Draw::DrawVec2Restart("SpotSystem");
    Draw::DrawTriangleRestart("SpotSystem");

    FOR_EACH_ENTITY(Spot, e)
        theEntityManager.DeleteEntity(e);
    }
    FOR_EACH_ENTITY(Block, e)
        theEntityManager.DeleteEntity(e);
    }


#if SAC_EMSCRIPTEN
    for (int i = 0; i < spots.size(); ++i) {
        Entity e = theEntityManager.CreateEntity("spot",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("spot"));
        TRANSFORM(e)->position = spots[i];
    }

    for (auto pair : walls) {
        Entity e = theEntityManager.CreateEntity("block",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (pair.first.first + pair.first.second) / 2.f;
        TRANSFORM(e)->size.x = glm::length(pair.first.second - pair.first.first);
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( pair.first.second - pair.first.first));
        BLOCK(e)->isDoubleFace = pair.second;
    }
#else
    std::ifstream myfile;

    if (filename.size() != 0) {
        myfile.open(filename);
    } else {
        myfile.open(currentLevelPath);
    }

    LOGE_IF( ! myfile.is_open(), "Could not open file '" << ((filename.size() != 0) ? filename : currentLevelPath) << "'");
    std::string line;

    std::getline (myfile, line);
    std::istringstream iss(line);
    int spotCount = 0;
    iss >> spotCount;
    while (--spotCount >= 0) {
        std::getline(myfile, line);
        iss.clear();
        iss.str(line);

        Entity e = theEntityManager.CreateEntity("spot",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("spot"));
        iss >> TRANSFORM(e)->position.x;
        iss.ignore(1, ',');
        iss >> TRANSFORM(e)->position.y;
    }

    while (std::getline(myfile, line)) {
        iss.clear();
        iss.str(line);

        glm::vec2 firstPoint, secondPoint;
        std::string isDoubleFace;

        iss >> firstPoint.x;
        iss.ignore(1, ',');
        iss >> firstPoint.y;

        iss.ignore(1, '|');

        iss >> secondPoint.x;
        iss.ignore(1, ',');
        iss >> secondPoint.y;

        iss >> isDoubleFace;

        Entity e = theEntityManager.CreateEntity("block",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (secondPoint + firstPoint) / 2.f;
        TRANSFORM(e)->size.x = glm::length(secondPoint - firstPoint);
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( secondPoint - firstPoint));
        BLOCK(e)->isDoubleFace = (isDoubleFace == "true");
    }

    myfile.close();
#endif
}
