#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "base/EntityManager.h"
#include "systems/AnchorSystem.h"
#include "systems/TextSystem.h"
#include "systems/ZSQDSystem.h"
#include "systems/BulletSystem.h"
#include "systems/UnitSystem.h"
#include "systems/WeaponSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/AutoDestroySystem.h"

#include "../MyTestGame.h"
#include "Scenes.h"
#include "../LoopHelper.h"

class BeginLoopScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
    int loop;

public:

    BeginLoopScene(MyTestGame* _game) : SceneState<Scene::Enum>("begin_loop", SceneEntityMode::InstantaneousOnEnter, SceneEntityMode::InstantaneousOnExit), game(_game) {}

    bool updatePreEnter(Scene::Enum, float) override {
        /* wait for all debris to disappear */
        return (theAutoDestroySystem.entityCount() == 0);
    }

    Scene::Enum update(float) override {
        LOGI("BEGIN LOOP");
        /* destroy all dynamic entities from level */
        /* bullet */  theBulletSystem.deleteAllEntities();
        /* weapon */  theWeaponSystem.deleteAllEntities();
        /* hitzone, body parts, etc */
        theUnitSystem.forEachECDo([] (Entity, UnitComponent* uc) -> void {
            if (uc->hitzone) theEntityManager.DeleteEntity(uc->hitzone);
            if (uc->head) theEntityManager.DeleteEntity(uc->head);
            if (uc->body) theEntityManager.DeleteEntity(uc->body);
        });
        /* units */   theUnitSystem.deleteAllEntities();

        game->aiUnits.clear();
        game->playerUnits.clear();

        /* recreate ai */
        {
            auto level = game->gameThreadContext->assetAPI->listAssetContent(".entity", "entities/level");
            for (auto l: level) {
                if (!strstr(l.c_str(), "ai")) continue;
                char tmp[128];
                sprintf(tmp, "level/%s", l.c_str());
                Entity enemy = theEntityManager.CreateEntityFromTemplate(tmp);

                TRANSFORM(enemy)->z = 0.0;
                MyTestGame::buildUnitParts(enemy);
                RENDERING(UNIT(enemy)->body)->color = Color(0.8, 0.8, 0);
                RENDERING(UNIT(enemy)->head)->color = Color(0.8, 0.0, 0.3);

                game->aiUnits.push_back(enemy);
            }
        }

        int playerCount = LoopHelper::playerCount();
        int activePlayerIndex = LoopHelper::activePlayerIndex();

        for (int i=0; i<playerCount; i++) {

            Entity newUnit = theEntityManager.CreateEntityFromTemplate("player");
            MyTestGame::buildUnitParts(newUnit);
            UNIT(newUnit)->index = i;
            ANCHOR(UNIT(newUnit)->body)->z = -0.4;
            TEXT(newUnit)->text = (char)('1' + i);
            TEXT(newUnit)->color = (i == activePlayerIndex) ? Color(1, 0, 0) : Color(0, 0, 0);
            game->playerUnits.push_back(newUnit);
        }

        /* active player unit is the latest one */
        game->playerUnit = game->playerUnits[activePlayerIndex];
        ZSQD(game->playerUnit)->lateralMove = false;
        TRANSFORM(game->camera)->position = TRANSFORM(game->playerUnit)->position;

        char tmp[128];
        sprintf(tmp, "Loop %d/%d - click when ready", activePlayerIndex + 1, playerCount);
        TEXT(e(HASH("begin_loop/get_ready_text", 0x3f1b44b5)))->text = tmp;
        return Scene::Game;
    }


};

namespace Scene {
    StateHandler<Scene::Enum>* CreateBeginLoopSceneHandler(MyTestGame* game) {
        return new BeginLoopScene(game);
    }
}
