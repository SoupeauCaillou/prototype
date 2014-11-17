#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

class EndLoopScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    EndLoopScene(MyTestGame* _game) : SceneState<Scene::Enum>("end_loop", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    Scene::Enum update(float) override {
        return Scene::EndLoop;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateEndLoopSceneHandler(MyTestGame* game) {
        return new EndLoopScene(game);
    }
}
