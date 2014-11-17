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

#include "systems/AISystem.h"
#include "systems/BulletSystem.h"
#include "systems/UnitSystem.h"
#include "systems/VisibilitySystem.h"
#include "systems/WeaponSystem.h"

#include "api/KeyboardInputHandlerAPI.h"
#include <SDL/SDL_keysym.h>
#include "util/Random.h"
#include "util/IntersectionUtil.h"
#include "base/PlacementHelper.h"

#include "base/StateMachine.inl"

void MyTestGame::init(const uint8_t*, int) {
    AISystem::CreateInstance();
    BulletSystem::CreateInstance();
    UnitSystem::CreateInstance();
    VisibilitySystem::CreateInstance();
    WeaponSystem::CreateInstance();

    glm::vec2 worldSize = PlacementHelper::ScreenSize * 2.0f;
    theCollisionSystem.worldSize = worldSize * 1.2f;

    sceneStateMachine = new StateMachine<Scene::Enum>();
    registerScenes(this, *sceneStateMachine);
    sceneStateMachine->setup(gameThreadContext->assetAPI);
    sceneStateMachine->start(Scene::Menu);
}

void MyTestGame::tick(float dt) {
    sceneStateMachine->update(dt);
}
