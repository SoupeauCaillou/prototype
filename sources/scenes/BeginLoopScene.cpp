#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

class BeginLoopScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:

    BeginLoopScene(MyTestGame* _game) : SceneState<Scene::Enum>("begin_loop", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    Scene::Enum update(float) override {
        return Scene::Game;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateBeginLoopSceneHandler(MyTestGame* game) {
        return new BeginLoopScene(game);
    }
}
