#include "HerdingDogGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/GridSystem.h"
#include "systems/CameraSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TextSystem.h"

#include "systems/MoveCommandSystem.h"
HerdingDogGame::HerdingDogGame() {
}

void HerdingDogGame::init(const uint8_t*, int) {
    MoveCommandSystem::CreateInstance();

    CAMERA(camera)->clearColor = Color(0,0,0);
    TRANSFORM(camera)->size = glm::vec2(28, 17);

    movesCount = theEntityManager.CreateEntityFromTemplate("hud/moves_count");
    homeButton = theEntityManager.CreateEntityFromTemplate("hud/home");

    registerScenes(this, sceneStateMachine);
    sceneStateMachine.setup(gameThreadContext->assetAPI);

    Scene::Enum start = Scene::Menu;
    level = NULL;
#if SAC_DESKTOP
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
            start = Scene::GameStart;
        }
    }
#endif
    sceneStateMachine.start(start);
}

bool HerdingDogGame::wantsAPI(ContextAPI::Enum api) const {
    switch (api) {
        case ContextAPI::Asset:
        return true;
        default:
        return false;
    }
}

void HerdingDogGame::tick(float dt) {
    sceneStateMachine.update(dt);
    switch (sceneStateMachine.getCurrentState()) {
    case Scene::GameStart:
        TEXT(movesCount)->show = true;
    case Scene::Editor:
        RENDERING(homeButton)->show = true;
        BUTTON(homeButton)->enabled = true;
        break;
    case Scene::Victory:
    case Scene::Lose:
        TEXT(movesCount)->show = false;
        RENDERING(homeButton)->show = false;
        BUTTON(homeButton)->enabled = false;
        break;
    default:
        break;
    }

    if (BUTTON(homeButton)->clicked) {
        TEXT(movesCount)->show = false;
        RENDERING(homeButton)->show = false;
        BUTTON(homeButton)->enabled = false;
        sceneStateMachine.forceNewState(Scene::Menu);
        sceneStateMachine.update(dt);
        theGridSystem.deleteAllEntities();
    }
}

#ifdef SAC_ANDROID
namespace std {
    template <class T> std::string to_string(T v) {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
}
#endif
void HerdingDogGame::updateMovesCount(int value) {
    movesCountV = value;
    TEXT(movesCount)->text = std::to_string(movesCountV);
}
