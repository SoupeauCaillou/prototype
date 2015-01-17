#include "SacHelloWorldGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/GridSystem.h"
#include "systems/CameraSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/GridSystem.h"

SacHelloWorldGame::SacHelloWorldGame() : grid(11,9,2.6) {
}

void SacHelloWorldGame::init(const uint8_t*, int) {
    CAMERA(camera)->clearColor = Color(0,0,0);
    TRANSFORM(camera)->size = glm::vec2(28, 17);

    registerScenes(this, sceneStateMachine);
    sceneStateMachine.setup(gameThreadContext->assetAPI);
    sceneStateMachine.start(Scene::Game);
}

bool SacHelloWorldGame::wantsAPI(ContextAPI::Enum api) const {
        switch (api) {
                case ContextAPI::Asset:
                        return true;
                default:
                        return false;
        }
}

void SacHelloWorldGame::tick(float dt) {
    sceneStateMachine.update(dt);
}
