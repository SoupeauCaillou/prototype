#include "LevelLoader.h"

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"

#include "base/PlacementHelper.h"

#include "util/DataFileParser.h"

static Entity createEntity(const DataFileParser & dfp, int number, const std::string & section) {
    std::stringstream ss;

    Entity e = theEntityManager.CreateEntityFromTemplate("game/" + section);

    ss << section << "_position_" << number;
    dfp.get(section, ss.str(), &TRANSFORM(e)->position.x, 2);
    ss.str(""); ss << section << "_size_" << number;
    dfp.get(section, ss.str(), &TRANSFORM(e)->size.x, 2);
    ss.str(""); ss << section << "_rotation_" << number;
    dfp.get(section, ss.str(), &TRANSFORM(e)->rotation, 1);
    

    TRANSFORM(e)->position = PlacementHelper::GimpPositionToScreen(TRANSFORM(e)->position);
    TRANSFORM(e)->size = PlacementHelper::GimpSizeToScreen(TRANSFORM(e)->size);
    TRANSFORM(e)->rotation = glm::radians(TRANSFORM(e)->rotation);

    LOGV(1, "One more '" << section << "' at pos:" << TRANSFORM(e)->position
         << " and size " << TRANSFORM(e)->size);

    return e;
}

void LevelLoader::init(AssetAPI* assetAPI) { 
    this->assetAPI = assetAPI;
}

void LevelLoader::load(const std::string & levelName) {
    FileBuffer fb = assetAPI->loadAsset("maps/" + levelName + ".ini");
    DataFileParser dfp;
    dfp.load(fb, levelName);

    dfp.get("", "objective_arrived", &objectiveArrived, 1);
    dfp.get("", "objective_survived", &objectiveSurvived, 1);
    dfp.get("", "objective_time_limit", &objectiveTimeLimit, 1);

    // create sheep
    for (unsigned i = 1; i <= dfp.sectionSize("sheep") / 3; ++i) {
        Entity e = createEntity(dfp, i, "sheep");

        sheep.push_back(e);
    }

    auto sheep = theAutonomousAgentSystem.RetrieveAllEntityWithComponent();

    //create bushes
    for (unsigned i = 1; i <= dfp.sectionSize("bush") / 3; ++i) {
        Entity bush = createEntity(dfp, i, "bush");
        for (auto s : sheep) {
            AUTONOMOUS(s)->obstacles.push_back(bush);
        }
        bushes.push_back(bush);
    }

    //create walls
    for (unsigned i = 1; i <= dfp.sectionSize("wall") / 3; ++i) {
        Entity wall = createEntity(dfp, i, "wall");

        for (auto s : sheep) {
            AUTONOMOUS(s)->obstacles.push_back(wall);
        };
        walls.push_back(wall);
    }

    //create final zone
    for (unsigned i = 1; i <= dfp.sectionSize("zone") / 3; ++i) {
         arrivalZone = createEntity(dfp, i, "zone");
    }

}
