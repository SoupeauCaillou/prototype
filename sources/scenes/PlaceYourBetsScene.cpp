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
#include "util/Random.h"
#include "PrototypeGame.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

struct PlaceYourBetsScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;

    PlaceYourBetsScene(PrototypeGame* game)
        : SceneState<Scene::Enum>(
              "PlaceYourBets", SceneEntityMode::DoNothing, SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void setup(AssetAPI*) override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        int pos[2];
        Random::N_Ints(2, pos, 0, MAZE_SIZE - 1);
        TRANSFORM(game->guy[0])->position = TRANSFORM(game->grid[pos[0]][pos[1]].e)->position;
        RENDERING(game->guy[0])->show = true;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        return Scene::InGame;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
};

namespace Scene {
    StateHandler<Scene::Enum>* CreatePlaceYourBetsSceneHandler(PrototypeGame* game) {
        return new PlaceYourBetsScene(game);
    }
}
