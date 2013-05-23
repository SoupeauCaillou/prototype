#include "LevelSystem.h"
#include "base/Log.h"
#include "base/Color.h"
#include "base/PlacementHelper.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include <fstream>
#include <sstream>

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
    std::string line;

    std::vector<Entity> blocks;

    LOGE_IF( ! myfile.is_open(), "Could not open file '" << filename << "'");

    int blockNumber = 0;


    glm::vec2 blockSize(0);

    int y = 1;
    while ( myfile.good() ) {
        std::getline (myfile, line);
        ++y;

        for (int x = 0; x < (int)line.size(); ++x) {
            blockSize.x = line.size() + 2;

            //ignore '.'
            if (line[x] == '.')
                continue;

            std::stringstream ss;
            ss << "block" << ++blockNumber;
            Entity e = theEntityManager.CreateEntity(ss.str(),
              EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));

            TRANSFORM(e)->position = glm::vec2(x + 1, - y);
            TRANSFORM(e)->size = glm::vec2(0.8f);
            switch (line[x]) {
                case 'x':
                    RENDERING(e)->color = Color(0.f, 0., 1.);
                    break;
                case 'o':
                    RENDERING(e)->color = Color(1.f, 0., 0.);
                    break;
            }
            blocks.push_back(e);
        }

    }
    blockSize.y = y;

    for (int wall = 0; wall < blockSize.x; ++wall) {
        //top wall
        Entity e = theEntityManager.CreateEntity("horizontalWall",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = glm::vec2(wall, -1);
        RENDERING(e)->color = Color(0,0,0);
        blocks.push_back(e);

        //bottom wall
        e = theEntityManager.CreateEntity("horizontalWall",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));

        TRANSFORM(e)->position = glm::vec2(wall, - blockSize.y);
        RENDERING(e)->color = Color(0,0,0);

        blocks.push_back(e);
    }

    for (int wall = 0; wall < blockSize.y; ++wall) {
        //left wall
        Entity e = theEntityManager.CreateEntity("verticalWall",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));
        TRANSFORM(e)->position = glm::vec2(0, - wall);
        RENDERING(e)->color = Color(0,0,0);
        blocks.push_back(e);

        //right wall
        e = theEntityManager.CreateEntity("verticalWall",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("block"));

        TRANSFORM(e)->position = glm::vec2(blockSize.x - 1, - wall);
        RENDERING(e)->color = Color(0,0,0);

        blocks.push_back(e);
    }

    glm::vec2 screenSize(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
    for (Entity e : blocks) {
        TRANSFORM(e)->size *= screenSize / blockSize;
        TRANSFORM(e)->position = .5f * glm::vec2(-screenSize.x, screenSize.y) + screenSize / blockSize * (.5f + TRANSFORM(e)->position);
    }


    myfile.close();
}
