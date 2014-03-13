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

    LOGI("one more " << section << " at pos:" << TRANSFORM(e)->position
         << " and size " << TRANSFORM(e)->size);


    return e;
}

void LevelLoader::load(AssetAPI* assetAPI, const std::string & levelName) {
    FileBuffer fb = assetAPI->loadAsset("maps/" + levelName + ".ini");
    DataFileParser dfp;
    dfp.load(fb, levelName);

    std::string attributeName;

    for (unsigned i = 1; i <= dfp.sectionSize("sheep") / 3; ++i) {
        createEntity(dfp, i, "sheep");
    }

    auto sheep = theAutonomousAgentSystem.RetrieveAllEntityWithComponent();

    //create bushes
    for (unsigned i = 1; i <= dfp.sectionSize("bush") / 3; ++i) {
        Entity bush = createEntity(dfp, i, "bush");
        for (auto s : sheep) {
            AUTONOMOUS(s)->obstacles.push_back(bush);
        }
    }

    //create walls
    for (unsigned i = 1; i <= dfp.sectionSize("wall") / 3; ++i) {
        Entity wall = createEntity(dfp, i, "wall");

        for (auto s : sheep) {
            AUTONOMOUS(s)->obstacles.push_back(wall);
        }
    }

    //create zones
    for (unsigned i = 1; i <= dfp.sectionSize("zone") / 3; ++i) {
        createEntity(dfp, i, "zone");
    }

}
