#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "base/EntityManager.h"
#include "systems/AISystem.h"
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
#include "../GameLogic.h"

class ObjectiveLoopScene : public SceneState<Scene::Enum> {
    MyTestGame* game;

public:

    ObjectiveLoopScene(MyTestGame* _game) : SceneState<Scene::Enum>("objective", SceneEntityMode::InstantaneousOnEnter, SceneEntityMode::InstantaneousOnExit), game(_game) {}

    bool updatePreEnter(Scene::Enum, float) override {
        /* wait for all debris to disappear */
        return (theAutoDestroySystem.entityCount() == 0);
    }

    Scene::Enum update(float) override {
        char tmp[256];

        int activePlayerIndex = LoopHelper::activePlayerIndex();
        Entity objective = e(HASH("objective/objective", 0xa280daa2));
        int dead = LoopHelper::unitToSaveFromDeath();
        if (dead >= 0) {
            sprintf(tmp, "Unit #%d: prevent unit #%d from dying at %.2f s", activePlayerIndex + 1, dead + 1, LoopHelper::unitDeathTime());
        } else {
            sprintf(tmp, "Unit #%d: kill all %d enemies", activePlayerIndex, theAISystem.entityCount());
        }
        TEXT(objective)->text = tmp;

        for (int i=0; i<(int)game->playerUnits.size(); i++) {
            TEXT(game->playerUnits[i])->color = (i == activePlayerIndex) ? Color(1, 0, 0) : Color(0, 0, 0);
        }

        return Scene::Game;
    }

    bool updatePreExit(Scene::Enum to, float dt) override {
        const int activePlayerIndex = LoopHelper::activePlayerIndex();
        bool fastforward = LoopHelper::canFastForward(activePlayerIndex);

        if (fastforward) {
            TEXT(e(HASH("objective/get_ready_text", 0xf9ea831f)))->text = "faaast forward";
            TEXT(e(HASH("objective/get_ready_text", 0xf9ea831f)))->color = Color(1, 0, 0);

            updateLogic(dt, game, true);
            for (auto* sys : game->orderedSystemsToUpdate) {
                sys->Update(dt);
            }
            updateLogic(dt, game, true);

            return false;
        }

        TEXT(e(HASH("objective/get_ready_text", 0xf9ea831f)))->text = "click when ready";
        TEXT(e(HASH("objective/get_ready_text", 0xf9ea831f)))->color = Color(1, 1, 1);

        TRANSFORM(game->camera)->position = TRANSFORM(game->playerUnits[activePlayerIndex])->position;
        return SceneState::updatePreExit(to, dt);
    }

};

namespace Scene {
    StateHandler<Scene::Enum>* CreateObjectiveSceneHandler(MyTestGame* game) {
        return new ObjectiveLoopScene(game);
    }
}
