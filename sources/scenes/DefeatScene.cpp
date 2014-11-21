#include "base/StateMachine.h"
#include "base/SceneState.h"

#include "../MyTestGame.h"
#include "Scenes.h"

#include "base/TouchInputManager.h"
#include "systems/BlinkSystem.h"
#include "systems/TextSystem.h"

class DefeatScene : public SceneState<Scene::Enum> {
    MyTestGame* game;
public:
    DefeatScene(MyTestGame* _game) : SceneState<Scene::Enum>("defeat", SceneEntityMode::Fading, SceneEntityMode::Fading), game(_game) {}

    void onEnter(Scene::Enum) override {
        Entity t = e(HASH("defeat/defeat_text", 0xbf9ad71));
        TEXT(t)->show = true;
    }

    Scene::Enum update(float) override {
        if (theTouchInputManager.hasClicked()) {
            return Scene::Menu;
        }
        return Scene::Defeat;
    }

    void onExit(Scene::Enum) override {
        Entity t = e(HASH("defeat/defeat_text", 0xbf9ad71));
        TEXT(t)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateDefeatSceneHandler(MyTestGame* game) {
        return new DefeatScene(game);
    }
}
