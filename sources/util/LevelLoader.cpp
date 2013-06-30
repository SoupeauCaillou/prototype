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
#include <iomanip>

#include <glm/gtx/vector_angle.hpp>

void LevelLoader::SaveInFile(const std::string & filename, const std::vector<Entity> & spotList,
    const std::vector<std::pair<Entity, Entity>> & wallList, const float objDistance) {
    //save in file each walls
    std::ofstream myfile (filename);
    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

    //spot number
    myfile << "nb_spot = " << spotList.size() << "\n";
    myfile << "nb_wall = " << wallList.size() << "\n\n";
    myfile << "objective_distance = " << objDistance << "\n\n";

    for (unsigned i = 0; i < spotList.size(); ++i) {
        myfile << "\n[spot_" << i+1 << "]\n";
        myfile << "position = " << std::setprecision(2) << TRANSFORM(spotList[i])->position << "\n";
    }


    for (unsigned i = 1; i <= wallList.size(); ++i) {
        myfile << "\n[wall_" << i << "]\n";
        myfile << "pos1 = " << std::setprecision(2) << TRANSFORM(wallList[i].first)->position << "\n";
        myfile << "pos2 = " << std::setprecision(2) << TRANSFORM(wallList[i].second)->position << "\n";
        myfile << "two_sided = false\n";
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
    // objective distance
    theSpotSystem.totalHighlightedDistance2Objective = 0.;
    dfp.get("", "objective_distance", &theSpotSystem.totalHighlightedDistance2Objective, 1, true);

    for (int i = 1; i <= spotCount; ++i) {
        std::stringstream section;
        section << "spot_" << i;

        Entity e = theEntityManager.CreateEntity(section.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("spot"));

        dfp.get(section.str(), "position", &TRANSFORM(e)->position.x, 2, true);
    }

    for (int i = 1; i <= wallCount; ++i) {
        std::stringstream section;
        section << "wall_" << i;

        glm::vec2 firstPoint, secondPoint;

        dfp.get(section.str(), "pos1", &firstPoint.x, 2, true);
        dfp.get(section.str(), "pos2", &secondPoint.x, 2, true);

        Entity e = theEntityManager.CreateEntity(section.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = (secondPoint + firstPoint) / 2.f;
        TRANSFORM(e)->size.x = glm::length(secondPoint - firstPoint);
        TRANSFORM(e)->size.y = 0.1;
        TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( secondPoint - firstPoint));

        dfp.get(section.str(), "two_sided", &BLOCK(e)->isDoubleFace, 1, true);
    }
    LOGI("Successfully loaded level " << ctx);
    return true;
}