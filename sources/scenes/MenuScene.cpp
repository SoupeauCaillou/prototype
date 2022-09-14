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

#include "base/EntityManager.h"
#include "base/SceneState.h"

#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

#include "PrototypeGame.h"

struct MenuScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;

    Entity bStart;

    MenuScene(PrototypeGame* game)
        : SceneState<Scene::Enum>("menu", SceneEntityMode::DoNothing, SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void setup(AssetAPI*) override {
        this->bStart = theEntityManager.CreateEntity(HASH("bStart", 0));
        ADD_COMPONENT(this->bStart, Transformation);
        ADD_COMPONENT(this->bStart, Button);
        ADD_COMPONENT(this->bStart, Rendering);
        TRANSFORM(this->bStart)->size = glm::vec2(5, 5);
        RENDERING(this->bStart)->color = Color(1, 0, 0);
        TRANSFORM(this->bStart)->z = 1.;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override { RENDERING(this->bStart)->show = true; }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(bStart)->clicked) {
            return Scene::GameStart;
        }

        return Scene::Menu;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        RENDERING(this->bStart)->show = false;
        BUTTON(this->bStart)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) { return new MenuScene(game); }
} // namespace Scene
