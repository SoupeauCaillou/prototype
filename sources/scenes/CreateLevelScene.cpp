#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

#include "systems/AISystem.h"
#include "systems/BulletSystem.h"
#include "systems/UnitSystem.h"
#include "systems/VisibilitySystem.h"
#include "systems/WeaponSystem.h"

#include "base/PlacementHelper.h"
#include "../LoopHelper.h"

class CreateLevelScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    CreateLevelScene(MyTestGame* _game) : SceneState<Scene::Enum>("create_level", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}


    Scene::Enum update(float) override {
        std::vector<Entity> blocks;
        auto level = game->gameThreadContext->assetAPI->listAssetContent(".entity", "entities/level");
        {
            for (auto l: level) {
                if (!strstr(l.c_str(), "block")) continue;
                char tmp[128];
                sprintf(tmp, "level/%s", l.c_str());
                Entity block = theEntityManager.CreateEntityFromTemplate(tmp);
                blocks.push_back(block);
            }
        }

        int aiCount = 0;
        for (auto l: level) {
            if (!strstr(l.c_str(), "ai")) continue;
            aiCount++;
        }
        LoopHelper::setAICount(aiCount);

        /* First loop ! */
        LoopHelper::start();

        return Scene::BeginLoop;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateCreateLevelSceneHandler(MyTestGame* game) {
        return new CreateLevelScene(game);
    }
}
