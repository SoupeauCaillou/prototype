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

void LevelLoader::SaveInFile(const std::string & filename, const std::vector<Entity> & spotList, const std::vector<Entity> & wallList) {
    //save in file each walls
    std::ofstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

    //spot number
    myfile << "nb_spot = " << spotList.size() << "\n";
    myfile << "nb_wall = " << wallList.size() << "\n\n";

    for (unsigned i = 0; i < spotList.size(); ++i) {
        myfile << "\n[spot_" << i << "]\n";
        myfile << "position = " << TRANSFORM(spotList[i])->position << "\n";
    }


    for (unsigned i = 0; i < wallList.size(); ++i) {
        auto * tc = TRANSFORM(wallList[i]);
        auto offset = glm::rotate(tc->size / 2.f, tc->rotation);

        myfile << "\n[wall_" << i << "]\n";
        myfile << "pos1 = " << tc->position - offset << "\n";
        myfile << "pos2 = " << tc->position + offset << "\n";
    }

    myfile.close();
}


bool LevelLoader::LoadFromFile(const std::string& ctx, const FileBuffer& fb) {
    DataFileParser dfp;
    if (!dfp.load(fb, ctx)) {
        LOGW("Couldn't load level " << ctx);
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
    LOGI("Successfully loaded level " << ctx);
    return true;
}
