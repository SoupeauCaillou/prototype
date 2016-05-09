/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "base/SceneState.h"

#include "PrototypeGame.h"
#include "PlayerSystem.h"
#include "systems/TransformationSystem.h"

struct MenuScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;

    MenuScene(PrototypeGame* game)
        : SceneState<Scene::Enum>(
              "menu", SceneEntityMode::DoNothing, SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void setup(AssetAPI*) override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        TRANSFORM(game->guy[0])->position = glm::vec2(0.0f);
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (PLAYER(game->guy[0])->input.actions[0] == InputState::Released)
            return Scene::InGame;
        return Scene::Menu;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
