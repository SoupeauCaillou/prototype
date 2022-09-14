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
#include "base/TimeUtil.h"

#include "systems/CameraSystem.h"

#include "PrototypeGame.h"

struct InGameScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;

    Entity tractor, bee, walls[4];

    InGameScene(PrototypeGame* game)
        : SceneState<Scene::Enum>("in_game", SceneEntityMode::DoNothing, SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void setup(AssetAPI*) override {
        this->tractor = theEntityManager.CreateEntityFromTemplate("game/tractor");
        this->bee = theEntityManager.CreateEntityFromTemplate("game/bee");
        this->walls[0] = theEntityManager.CreateEntityFromTemplate("game/wall_east");
        this->walls[1] = theEntityManager.CreateEntityFromTemplate("game/wall_north");
        this->walls[2] = theEntityManager.CreateEntityFromTemplate("game/wall_west");
        this->walls[3] = theEntityManager.CreateEntityFromTemplate("game/wall_south");
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        float c = glm::abs(glm::cos(TimeUtil::GetTime()));
        CAMERA(this->game->camera)->clearColor = Color(c, c, c);

        for (auto wall : this->walls) {
            RENDERING(wall)->show = true;
            break;
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override { return Scene::InGame; }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateInGameSceneHandler(PrototypeGame* game) {
        return new InGameScene(game);
    }
}
