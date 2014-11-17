#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

class GameScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    GameScene(MyTestGame* _game) : SceneState<Scene::Enum>("game", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    Scene::Enum update(float) override {
        return Scene::Game;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameSceneHandler(MyTestGame* game) {
        return new GameScene(game);
    }
}
