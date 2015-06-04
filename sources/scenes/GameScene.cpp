#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"
#include "systems/AISystem.h"
#include "systems/BulletSystem.h"
#include "systems/UnitSystem.h"
#include "systems/VisibilitySystem.h"
#include "systems/WeaponSystem.h"

#include "base/TouchInputManager.h"

#include "systems/AnchorSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"

#include "api/KeyboardInputHandlerAPI.h"
// #include <SDL/SDL_keysym.h>
#include "../LoopHelper.h"
#include "../GameLogic.h"

class GameScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    GameScene(MyTestGame* _game) : SceneState<Scene::Enum>("game", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    void onPreEnter(Scene::Enum from) override {
        theAISystem.forEachECDo([] (Entity e, AIComponent* ac) -> void {
            ac->_targetAngle = TRANSFORM(e)->rotation;
        });
        SceneState<Scene::Enum>::onPreEnter(from);
    }

    bool updatePreEnter(Scene::Enum from, float dt) override {
        bool b = SceneState<Scene::Enum>::updatePreEnter(from, dt);

        return b &&
            theTouchInputManager.hasClicked();
    }

    void onEnter(Scene::Enum from) override {

        SceneState<Scene::Enum>::onEnter(from);
    }

    Scene::Enum update(float dt) override {
        char tmp[128];

        if (LoopHelper::unitToSaveFromDeath() < 0) {
            sprintf(tmp, "%.2f s", LoopHelper::loopDuration());
        } else {
            sprintf(tmp, "%.2f s", glm::max(0.0f, LoopHelper::unitDeathTime() - LoopHelper::loopDuration()));
        }
        TEXT(e(HASH("game/chrono_text", 0x900a8522)))->text = tmp;

        /* if at least one player unit is dead, loop failure */
        int alive = 0;
        for (auto u: game->playerUnits) {
            if (!UNIT(u)->alive) {
                if (UNIT(u)->index == 0 && LoopHelper::activePlayerIndex() == 0) {
                    LOGI("Unit '" << u << "' is dead. Looping");
                    LoopHelper::loopFailedUnitDead(UNIT(u)->index);
                    LOGI("LOOP FAILED");
                    return Scene::BeginLoop;
                }
            } else {
                alive ++;
            }
        }
        if (alive == 0)
            return Scene::Defeat;

        if (LoopHelper::playerCount() > 1 && LoopHelper::isLoopLongerThanPrevious()) {
            LoopHelper::loopSucceeded();
            LOGI("LOOP SUCCESSFULL");
            return Scene::Objective;
        }

        /* if all enemy units are dead -> victory */

        int aiAliveCount = 0;
        for (auto u: game->aiUnits) {
            aiAliveCount += (UNIT(u)->alive);
        }
        {
            char tmp[128];
            sprintf(tmp, "Unit #%d - enemy left: %d/%d", LoopHelper::activePlayerIndex() + 1, aiAliveCount, 7);
            TEXT(e(HASH("game/loop_text", 0x35373a8a)))->text = tmp;
        }
        if (aiAliveCount == 0) {
            LOGI("LOOP VICTORY");
            return Scene::Victory;
        }

        updateLogic(dt, game, false);

        return Scene::Game;
    }

    void onExit(Scene::Enum to) override {
        SceneState<Scene::Enum>::onExit(to);
        TEXT(e(HASH("game/loop_text", 0x35373a8a)))->text = "";
        TEXT(e(HASH("game/chrono_text", 0x900a8522)))->text = "";

    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameSceneHandler(MyTestGame* game) {
        return new GameScene(game);
    }
}
