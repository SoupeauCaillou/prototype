#include "MyTestGame.h"
#include "scenes/Scenes.h"
#include "systems/AISystem.h"
#include "systems/BulletSystem.h"
#include "systems/UnitSystem.h"
#include "systems/VisibilitySystem.h"
#include "systems/VisionSystem.h"
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
#include <SDL/SDL_keysym.h>
#include "LoopHelper.h"

struct InputInterface {
    virtual bool moveUp() = 0;
    virtual bool moveDown() = 0;
    virtual bool moveLeft() = 0;
    virtual bool moveRight() = 0;

    virtual bool shootWeapon1() = 0;
    virtual bool shootWeapon2() = 0;

    virtual glm::vec2 lookat() = 0;
};

struct RealInput : public InputInterface {
    KeyboardInputHandlerAPI* kb;

    RealInput(KeyboardInputHandlerAPI* k) : kb(k) {}

    bool moveUp() { return kb->isKeyPressed(SDLK_z); }
    bool moveDown() { return kb->isKeyPressed(SDLK_s); }
    bool moveLeft() { return kb->isKeyPressed(SDLK_q); }
    bool moveRight() { return kb->isKeyPressed(SDLK_d); }

    bool shootWeapon1() { return theTouchInputManager.isTouched(0); }
    bool shootWeapon2() { return theTouchInputManager.isTouched(1); }

    glm::vec2 lookat() { return theTouchInputManager.getOverLastPosition(); }
};

struct ReplayInput : public InputInterface {
    int idx;
    void setPlayerIndex(int playerIndex) { idx = playerIndex; }

    bool moveUp() { return LoopHelper::input(Input::Up, idx); }
    bool moveDown() { return LoopHelper::input(Input::Down, idx); }
    bool moveLeft() { return LoopHelper::input(Input::Left, idx); }
    bool moveRight() { return LoopHelper::input(Input::Right, idx); }

    bool shootWeapon1() { return LoopHelper::input(Input::Left_Btn, idx); }
    bool shootWeapon2() { return LoopHelper::input(Input::Right_Btn, idx); }

    glm::vec2 lookat() { return LoopHelper::over(idx); }
};

void updateLogic(float dt, MyTestGame* game, bool fastforward) {

    const int activePlayerIndex = LoopHelper::activePlayerIndex();

    theAISystem.Update(dt);
    theBulletSystem.Update(dt);
    theUnitSystem.Update(dt);
    theVisibilitySystem.Update(dt);
    theVisionSystem.Update(dt);
    theWeaponSystem.Update(dt);

    RealInput realInput(game->gameThreadContext->keyboardInputHandlerAPI);
    ReplayInput replayInput;

    for (int i=0; i<(int)game->playerUnits.size(); i++) {
        Entity unit = game->playerUnits[i];

        /* Pick proper input source */
        InputInterface* input;
        if (i == activePlayerIndex && !fastforward) {
            input = &realInput;

            /* save input */
            if (input->moveUp()) LoopHelper::save(Input::Up, i);
            if (input->moveDown()) LoopHelper::save(Input::Down, i);
            if (input->moveLeft()) LoopHelper::save(Input::Left, i);
            if (input->moveRight()) LoopHelper::save(Input::Right, i);
            if (input->shootWeapon1()) LoopHelper::save(Input::Left_Btn, i);
            if (input->shootWeapon2()) LoopHelper::save(Input::Right_Btn, i);
            LoopHelper::save(input->lookat(), i);
        } else {
            input = &replayInput;
            replayInput.setPlayerIndex(i);
        }

        if (i == activePlayerIndex) {
            /* and move camera */
            glm::vec2 target;
            target = (3.0f * TRANSFORM(unit)->position + input->lookat()) / 4.0f;
            glm::vec2 diff = target - TRANSFORM(game->camera)->position;
            TRANSFORM(game->camera)->position += diff * dt * 8.0f;
        }

        float angleHead;
        {
            Entity head = UNIT(unit)->head;
            glm::vec2 diff = input->lookat() - TRANSFORM(head)->position;
            angleHead = glm::atan(diff.y, diff.x);
            ANCHOR(UNIT(unit)->head)->rotation = angleHead - TRANSFORM(UNIT(unit)->body)->rotation;
        }
        for (int i=0; i<2; i++) {
            Entity weapon = UNIT(unit)->weapon[i];
            glm::vec2 diff = input->lookat() - TRANSFORM(weapon)->position;
            float angleWeapon = glm::atan(diff.y, diff.x);
            ANCHOR(weapon)->rotation = angleWeapon - angleHead;
        }

        if (input->moveUp()) {
            ZSQD(unit)->addDirectionVector(glm::vec2(0.0f, 1.0f));
        } else if (input->moveDown()) {
            ZSQD(unit)->addDirectionVector(glm::vec2(0.0f, -1.0f));
        }

        if (input->moveLeft()) {
            ZSQD(unit)->addDirectionVector(glm::vec2(-1.0f, 0.0f));
        } else if (input->moveRight()) {
            ZSQD(unit)->addDirectionVector(glm::vec2(1.0f, 0.0f));
        }

        WEAPON(UNIT(unit)->weapon[0])->fire = input->shootWeapon1();
        WEAPON(UNIT(unit)->weapon[1])->fire = input->shootWeapon2();
    }

    LoopHelper::update(dt, fastforward);
}
