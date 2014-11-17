#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

#include "base/EntityManager.h"
#include "systems/AnchorSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"

#include "systems/AISystem.h"
#include "systems/BulletSystem.h"
#include "systems/UnitSystem.h"
#include "systems/VisibilitySystem.h"
#include "systems/WeaponSystem.h"

#include "base/PlacementHelper.h"
#include "util/Random.h"

class CreateLevelScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    CreateLevelScene(MyTestGame* _game) : SceneState<Scene::Enum>("create_level", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}


    Scene::Enum update(float) override {
            game->playerUnit = theEntityManager.CreateEntityFromTemplate("player");
            buildUnitParts(game->playerUnit);
            ZSQD(game->playerUnit)->lateralMove = false;

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

            {
                for (auto l: level) {
                    if (!strstr(l.c_str(), "ai")) continue;
                    char tmp[128];
                    sprintf(tmp, "level/%s", l.c_str());
                    Entity enemy = theEntityManager.CreateEntityFromTemplate(tmp);

                    TRANSFORM(enemy)->z = 0.0;
                    TRANSFORM(enemy)->rotation = Random::Float(0, 6.28);
                    buildUnitParts(enemy);
                    RENDERING(UNIT(enemy)->body)->color = Color(0.8, 0.8, 0);
                    RENDERING(UNIT(enemy)->head)->color = Color(0.8, 0.0, 0.3);
                }
            }

        return Scene::BeginLoop;
    }

private:
    void buildUnitParts(Entity unit) {
        UNIT(unit)->body = theEntityManager.CreateEntityFromTemplate("body");
        UNIT(unit)->head = theEntityManager.CreateEntityFromTemplate("head");
        UNIT(unit)->weapon[0] = theEntityManager.CreateEntityFromTemplate("gun");
        UNIT(unit)->weapon[1] = theEntityManager.CreateEntityFromTemplate("machinegun");
        UNIT(unit)->hitzone = theEntityManager.CreateEntityFromTemplate("hitzone");

        ANCHOR(UNIT(unit)->body)->parent = unit;
        ANCHOR(UNIT(unit)->head)->parent = UNIT(unit)->body;
        ANCHOR(UNIT(unit)->hitzone)->parent = UNIT(unit)->head;
        ANCHOR(UNIT(unit)->weapon[0])->parent = UNIT(unit)->head;
        ANCHOR(UNIT(unit)->weapon[1])->parent = UNIT(unit)->head;
        ANCHOR(UNIT(unit)->weapon[1])->position.y = -ANCHOR(UNIT(unit)->weapon[1])->position.y;
    }

};

namespace Scene {
    StateHandler<Scene::Enum>* CreateCreateLevelSceneHandler(MyTestGame* game) {
        return new CreateLevelScene(game);
    }
}
