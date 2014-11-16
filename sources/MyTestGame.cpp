#include "MyTestGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"

#include "systems/AnchorSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"

#include "systems/BulletSystem.h"
#include "systems/UnitSystem.h"
#include "systems/VisibilitySystem.h"
#include "systems/WeaponSystem.h"

#include "api/KeyboardInputHandlerAPI.h"
#include <SDL/SDL_keysym.h>
#include "util/Random.h"
#include "util/IntersectionUtil.h"
#include "base/PlacementHelper.h"
Entity playerUnit;

void buildUnitParts(Entity unit) {
    UNIT(unit)->body = theEntityManager.CreateEntityFromTemplate("body");
    UNIT(unit)->head = theEntityManager.CreateEntityFromTemplate("head");
    UNIT(unit)->weapon = theEntityManager.CreateEntityFromTemplate("weapon");
    UNIT(unit)->hitzone = theEntityManager.CreateEntityFromTemplate("hitzone");

    ANCHOR(UNIT(unit)->body)->parent = unit;
    ANCHOR(UNIT(unit)->head)->parent = UNIT(unit)->body;
    ANCHOR(UNIT(unit)->hitzone)->parent = UNIT(unit)->head;
    ANCHOR(UNIT(unit)->weapon)->parent = UNIT(unit)->head;
}

void MyTestGame::init(const uint8_t*, int) {
    BulletSystem::CreateInstance();
    UnitSystem::CreateInstance();
    VisibilitySystem::CreateInstance();
    WeaponSystem::CreateInstance();

    playerUnit = theEntityManager.CreateEntityFromTemplate("player");
    buildUnitParts(playerUnit);
    ZSQD(playerUnit)->lateralMove = false;

    glm::vec2 worldSize = PlacementHelper::ScreenSize * 2.0f;

    std::vector<Entity> blocks;
    auto level = gameThreadContext->assetAPI->listAssetContent(".entity", "entities/level");
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

    theCollisionSystem.worldSize = worldSize * 1.2f;
}

void MyTestGame::tick(float dt) {
    theBulletSystem.Update(dt);
    theUnitSystem.Update(dt);
    theVisibilitySystem.Update(dt);
    theWeaponSystem.Update(dt);

    float angleHead;

    {
        Entity head = UNIT(playerUnit)->head;
        glm::vec2 diff = theTouchInputManager.getOverLastPosition() - TRANSFORM(head)->position;
        angleHead = glm::atan(diff.y, diff.x);
        ANCHOR(UNIT(playerUnit)->head)->rotation = angleHead - TRANSFORM(UNIT(playerUnit)->body)->rotation;
    }
    {
        Entity weapon = UNIT(playerUnit)->weapon;
        glm::vec2 diff = theTouchInputManager.getOverLastPosition() - TRANSFORM(weapon)->position;
        float angleWeapon = glm::atan(diff.y, diff.x);
        ANCHOR(weapon)->rotation = angleWeapon - angleHead;
    }

    ZSQD(playerUnit)->rotateToFaceDirection = true;
    //TRANSFORM(playerUnit)->rotation = angleHead;

    auto* kb = gameThreadContext->keyboardInputHandlerAPI;
    if (kb->isKeyPressed(Key::ByName(SDLK_z))) {
        // ZSQD(playerUnit)->rotateToFaceDirection = true;
        ZSQD(playerUnit)->addDirectionVector(glm::vec2(0.0f, 1.0f));
    } else if (kb->isKeyPressed(Key::ByName(SDLK_s))) {
        // ZSQD(playerUnit)->rotateToFaceDirection = false;
        ZSQD(playerUnit)->addDirectionVector(glm::vec2(0.0f, -1.0f));
    }

    if (kb->isKeyPressed(Key::ByName(SDLK_q))) {
        // ZSQD(playerUnit)->rotateToFaceDirection = false;
        ZSQD(playerUnit)->addDirectionVector(glm::vec2(-1.0f, 0.0f));
    } else if (kb->isKeyPressed(Key::ByName(SDLK_d))) {
        // ZSQD(playerUnit)->rotateToFaceDirection = false;
        ZSQD(playerUnit)->addDirectionVector(glm::vec2(1.0f, 0.0f));
    }

    WEAPON(UNIT(playerUnit)->weapon)->fire = theTouchInputManager.isTouched();

    // move camera
    glm::vec2 target;
    if (true || ZSQD(playerUnit)->currentSpeed <= 0) {
        target = (3.0f * TRANSFORM(playerUnit)->position + theTouchInputManager.getOverLastPosition()) / 4.0f;
    } else {
        target = TRANSFORM(playerUnit)->position + ZSQD(playerUnit)->currentDirection * ZSQD(playerUnit)->currentSpeed * 0.2f;
    }
    glm::vec2 diff = target - TRANSFORM(camera)->position;
    TRANSFORM(camera)->position += diff * dt * 8.0f;
}
