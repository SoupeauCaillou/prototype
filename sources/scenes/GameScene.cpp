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
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"

#include "api/KeyboardInputHandlerAPI.h"
#include <SDL/SDL_keysym.h>

class GameScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    GameScene(MyTestGame* _game) : SceneState<Scene::Enum>("game", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    Scene::Enum update(float dt) override {

        theAISystem.Update(dt);
        theBulletSystem.Update(dt);
        theUnitSystem.Update(dt);
        theVisibilitySystem.Update(dt);
        theWeaponSystem.Update(dt);

        float angleHead;

        if (!UNIT(game->playerUnit)->alive) {
            return Scene::EndLoop;
        }

        {
            Entity head = UNIT(game->playerUnit)->head;
            glm::vec2 diff = theTouchInputManager.getOverLastPosition() - TRANSFORM(head)->position;
            angleHead = glm::atan(diff.y, diff.x);
            ANCHOR(UNIT(game->playerUnit)->head)->rotation = angleHead - TRANSFORM(UNIT(game->playerUnit)->body)->rotation;
        }
        for (int i=0; i<2; i++) {
            Entity weapon = UNIT(game->playerUnit)->weapon[i];
            glm::vec2 diff = theTouchInputManager.getOverLastPosition() - TRANSFORM(weapon)->position;
            float angleWeapon = glm::atan(diff.y, diff.x);
            ANCHOR(weapon)->rotation = angleWeapon - angleHead;
        }

        ZSQD(game->playerUnit)->rotateToFaceDirection = true;

        auto* kb = game->gameThreadContext->keyboardInputHandlerAPI;
        if (kb->isKeyPressed(Key::ByName(SDLK_z))) {
            ZSQD(game->playerUnit)->addDirectionVector(glm::vec2(0.0f, 1.0f));
        } else if (kb->isKeyPressed(Key::ByName(SDLK_s))) {
            ZSQD(game->playerUnit)->addDirectionVector(glm::vec2(0.0f, -1.0f));
        }

        if (kb->isKeyPressed(Key::ByName(SDLK_q))) {
            ZSQD(game->playerUnit)->addDirectionVector(glm::vec2(-1.0f, 0.0f));
        } else if (kb->isKeyPressed(Key::ByName(SDLK_d))) {
            ZSQD(game->playerUnit)->addDirectionVector(glm::vec2(1.0f, 0.0f));
        }

        for (int i=0; i<2; i++) {
            WEAPON(UNIT(game->playerUnit)->weapon[i])->fire = theTouchInputManager.isTouched(i);
        }

        // move camera
        glm::vec2 target;
        if (true || ZSQD(game->playerUnit)->currentSpeed <= 0) {
            target = (3.0f * TRANSFORM(game->playerUnit)->position + theTouchInputManager.getOverLastPosition()) / 4.0f;
        } else {
            target = TRANSFORM(game->playerUnit)->position + ZSQD(game->playerUnit)->currentDirection * ZSQD(game->playerUnit)->currentSpeed * 0.2f;
        }
        glm::vec2 diff = target - TRANSFORM(game->camera)->position;
        TRANSFORM(game->camera)->position += diff * dt * 8.0f;

        return Scene::Game;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameSceneHandler(MyTestGame* game) {
        return new GameScene(game);
    }
}
