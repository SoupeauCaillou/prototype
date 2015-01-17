/*
    This file is part of RecursiveRunner.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    RecursiveRunner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    RecursiveRunner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "base/StateMachine.h"
#include "Scenes.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"

#include "systems/ADSRSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/MusicSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/AnchorSystem.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../SacHelloWorldGame.h"

#include "base/SceneState.h"


class GameScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;

    public:

    GameScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("game", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    void onPreEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreEnter(f);
    }

    void onEnter(Scene::Enum f) {
        SceneState<Scene::Enum>::onEnter(f);
    }

    Scene::Enum update(float) {
        return Scene::Game;
    }

    void onPreExit(Scene::Enum f) {
        SceneState<Scene::Enum>::onPreExit(f);
    }

    void onExit(Scene::Enum to) {
        SceneState<Scene::Enum>::onExit(to);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameSceneHandler(SacHelloWorldGame* game) {
        return new GameScene(game);
    }
}
