#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

#include "base/TouchInputManager.h"
#include "systems/BlinkSystem.h"
#include "systems/TextSystem.h"

class VictoryScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    VictoryScene(MyTestGame* _game) : SceneState<Scene::Enum>("victory", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    void onEnter(Scene::Enum) override {
        Entity t = e(HASH("victory/victory_text", 0x787eb88e));
        TEXT(t)->show = true;
    }

    Scene::Enum update(float) override {
        if (theTouchInputManager.hasClicked()) {
            return Scene::Menu;
        }
        return Scene::Victory;
    }

    void onExit(Scene::Enum) override {
        Entity t = e(HASH("victory/victory_text", 0x787eb88e));
        TEXT(t)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateVictorySceneHandler(MyTestGame* game) {
        return new VictoryScene(game);
    }
}
