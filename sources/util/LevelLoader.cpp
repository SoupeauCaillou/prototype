#include "LevelLoader.h"
#include "base/Log.h"
#include "base/Color.h"
#include "base/PlacementHelper.h"

#include "util/DrawSomething.h"
#include "util/DataFileParser.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/BlockSystem.h"
#include "systems/SpotSystem.h"

#include <fstream>
#include <sstream>

#include <glm/gtx/vector_angle.hpp>

#if SAC_EMSCRIPTEN
std::vector<std::pair<std::pair<glm::vec2, glm::vec2>, bool>> walls;
std::vector<glm::vec2> spots;
void LevelLoader::SaveInFile(const std::string &, const std::list<Entity> & wallList, const std::list<Entity> & spotList) {
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
void LevelLoader::SaveInFile(const std::string & filename, const std::vector<Entity> & spotList, const std::vector<Entity> & wallList) {
    //save in file each walls
    std::ofstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

    //spot number
    myfile << "nb_spot = " << spotList.size() << "\n";
    myfile << "nb_wall = " << wallList.size() << "\n\n";

    for (int i = 0; i < spotList.size(); ++i) {
        myfile << "\n[spot_" << i << "]\n";
        myfile << "position = " << TRANSFORM(spotList[i])->position << "\n";
    }


    for (int i = 0; i < wallList.size(); ++i) {
        auto * tc = TRANSFORM(wallList[i]);
        auto offset = glm::rotate(tc->size / 2.f, tc->rotation);

        myfile << "\n[wall_" << i << "]\n";
        myfile << "pos1 = " << tc->position - offset << "\n";
        myfile << "pos2 = " << tc->position + offset << "\n";
    }

    myfile.close();
}
#endif


bool LevelLoader::LoadFromFile(const std::string& ctx, const FileBuffer& fb) {
    DataFileParser dfp;
    if (!dfp.load(fb, ctx)) {
        return false;
    }
    // spot count
    int spotCount = 1;
    dfp.get("", "nb_spot", &spotCount, 1, true);
    // wall count
    int wallCount = 1;
    dfp.get("", "nb_wall", &wallCount, 1, true);

    for (int i = 0; i < spotCount; ++i) {
        std::stringstream ss;
        ss << "spot_" << i;

        Entity e = theEntityManager.CreateEntity("spot",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("spot"));

        dfp.get(ss.str(), "position", &TRANSFORM(e)->position.x, 2, true);
    }

    for (int i = 0; i < wallCount; ++i) {
        std::stringstream ss;
        ss << "wall_" << i;

        glm::vec2 firstPoint, secondPoint;

        dfp.get(ss.str(), "pos1", &firstPoint.x, 2, true);
        dfp.get(ss.str(), "pos2", &secondPoint.x, 2, true);

        Entity e = theEntityManager.CreateEntity("block",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (secondPoint + firstPoint) / 2.f;
        TRANSFORM(e)->size.x = glm::length(secondPoint - firstPoint);
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( secondPoint - firstPoint));

        dfp.get(ss.str(), "two_sided", &BLOCK(e)->isDoubleFace, 1, true);
    }

    return true;
}

/*
void LevelLoader::LoadFromFile(const std::string & filename) {
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
}*/
