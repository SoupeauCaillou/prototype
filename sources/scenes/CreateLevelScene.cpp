#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

class CreateLevelScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    CreateLevelScene(MyTestGame* _game) : SceneState<Scene::Enum>("create_level", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    Scene::Enum update(float) override {
        return Scene::CreateLevel;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateCreateLevelSceneHandler(MyTestGame* game) {
        return new CreateLevelScene(game);
    }
}
