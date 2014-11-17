#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

class VictoryScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    VictoryScene(MyTestGame* _game) : SceneState<Scene::Enum>("victory", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    Scene::Enum update(float) override {
        return Scene::Menu;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateVictorySceneHandler(MyTestGame* game) {
        return new VictoryScene(game);
    }
}
