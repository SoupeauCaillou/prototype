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
#include "gen/Scenes.h"

#include "systems/GridSystem.h"

#include "../SacHelloWorldGame.h"

#include "base/SceneState.h"


class CheckVictoryScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;

    public:

    CheckVictoryScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("check_victory", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    Scene::Enum update(float) {
        bool allSheepOnEndingCell = true;

        game->grid->forEachCellDo([this, &allSheepOnEndingCell] (const GridPos& pos) -> void {
            bool thereIsASheep = false;
            bool thereIsAnEnd = false;
            for (Entity e : game->grid->getEntitiesAt(pos)) {
                bitfield_t type = GRID(e)->type;

                thereIsASheep |= type & Case::Sheep;
                thereIsAnEnd |= type & Case::End;
            }
            if (thereIsASheep && !thereIsAnEnd) {
                allSheepOnEndingCell = false;
            }
        });

        if (allSheepOnEndingCell)
            return Scene::Victory;
        else
            return Scene::Game;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateCheckVictorySceneHandler(SacHelloWorldGame* game) {
        return new CheckVictoryScene(game);
    }
}
