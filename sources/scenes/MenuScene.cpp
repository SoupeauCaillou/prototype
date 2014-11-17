#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

class MenuScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    MenuScene(MyTestGame* _game) : SceneState<Scene::Enum>("menu", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    Scene::Enum update(float) override {
        return Scene::CreateLevel;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(MyTestGame* game) {
        return new MenuScene(game);
    }
}
