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

SacHelloWorldGame::SacHelloWorldGame() {
}

void SacHelloWorldGame::init(const uint8_t*, int) {
    CAMERA(camera)->clearColor = Color(0,0,0);
    TRANSFORM(camera)->size = glm::vec2(28, 17);

    registerScenes(this, sceneStateMachine);
    sceneStateMachine.setup(gameThreadContext->assetAPI);

    Scene::Enum start = Scene::Game;
    level = NULL;
    for (int i=1; i<arg.c; i++) {
        if (strcmp("-e", arg.v[i]) == 0 ||
            strcmp("--editor", arg.v[i]) == 0) {
            start = Scene::Editor;
            if (i < (arg.c - 1)) {
                level = arg.v[i+1];
                i++;
            }
        } else {
            level = arg.v[i];
        }
    }
    sceneStateMachine.start(start);
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
