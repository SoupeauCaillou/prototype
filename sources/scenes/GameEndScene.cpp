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

#include "base/StateMachine.h"
#include "Scenes.h"

#include "base/EntityManager.h"

#include "PrototypeGame.h"

struct GameEndScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    GameEndScene(PrototypeGame* game) : StateHandler<Scene::Enum>("GameEnd") {
        this->game = game;
    }

    void setup(AssetAPI*) override {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        for (auto e : game->levelLoader.zones) {
            theEntityManager.DeleteEntity(e);
        }
        game->levelLoader.zones.clear();
        for (auto e : game->levelLoader.sheep) {
            theEntityManager.DeleteEntity(e);
        }
        game->levelLoader.sheep.clear();
        for (auto e : game->levelLoader.walls) {
            theEntityManager.DeleteEntity(e);
        }
        game->levelLoader.walls.clear();
        for (auto e : game->levelLoader.bushes) {
            theEntityManager.DeleteEntity(e);
        }
        game->levelLoader.bushes.clear();
        theEntityManager.DeleteEntity(game->levelLoader.background);


        // save progression in file
        game->saveManager.save();
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameEndSceneHandler(PrototypeGame* game) {
        return new GameEndScene(game);
    }
}
