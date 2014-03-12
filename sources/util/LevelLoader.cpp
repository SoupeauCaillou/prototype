#include "LevelLoader.h"

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"

#include "util/DataFileParser.h"

void LevelLoader::load(const std::string & levelFilepath) {
    DataFileParser dfp(FileBuffer(levelFilepath));

    int count = 1;
    for (int i = 0; i < count; i++) {
        Entity e = theEntityManager.CreateEntityFromTemplate("game/sheep");
        TRANSFORM(e)->position.x += i;
    }

    auto sheep = theAutonomousAgentSystem.RetrieveAllEntityWithComponent();

    // not very well optimized...
    for (auto s : sheep) {
        for (auto s2 : sheep) {
            if (s2 != s) {
                AUTONOMOUS(s)->obstacles.push_back(s2);
            }
        }
    }

    //create obstacles
    for (int i = 0; i < 4; i++) {
        Entity e = theEntityManager.CreateEntityFromTemplate("game/wall");
        TRANSFORM(e)->position = glm::rotate(TRANSFORM(e)->position, glm::radians(90.f * i));
        TRANSFORM(e)->rotation = glm::radians(90.f * i);

        for (auto s : sheep) {
            AUTONOMOUS(s)->obstacles.push_back(e);
        }

    }
    Entity bush = theEntityManager.CreateEntityFromTemplate("game/bush");
    TRANSFORM(bush)->position = glm::vec2(0.f);
    TRANSFORM(bush)->size = glm::vec2(1.f);
    for (auto s : sheep) {
        AUTONOMOUS(s)->obstacles.push_back(bush);
    }
}
