/*
    This file is part of Prototype.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer

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
#include "systems/PlayerSystem.h"
#include "systems/SoldierSystem.h"
#include "PrototypeGame.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/VisionSystem.h"

struct BeginTurnScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    // Scene variables


    BeginTurnScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~BeginTurnScene() {}

    // Setup internal var, states, ...
    void setup() override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreEnter(Scene::Enum) override {
        TEXT(game->banner)->color = Color(0, 1, 0);

        for (auto wall: game->walls) {
            RENDERING(wall)-> show = true;
        }

        for (auto o: game->objs) {
            RENDERING(o)->show = true;
        }
        theSoldierSystem.forEachECDo([] (Entity e, SoldierComponent* sc) -> void {
            RENDERING(e)-> show = true;
            TEXT(sc->apIndicator)-> show = true;
        });

        RENDERING(game->background)->show = true;

        // quick test
        game->grid.doForEachCell([this] (const GridPos& p) -> void {
            std::list<Entity>& l = this->game->grid.getEntitiesAt(p);
            for (auto& e: l) {
                RENDERING(e)->show = true;
            }
        });

        game->grid.autoAssignEntitiesToCell(game->players);
        game->grid.autoAssignEntitiesToCell(game->yEnnemies);
        game->grid.autoAssignEntitiesToCell(game->bEnnemies);

        theVisionSystem.Update(0);
        game->visibilityManager.updateVisibility(game->players);
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float ) override {
        PLAYER(game->humanPlayer)->actionPointsLeft =
            PLAYER(game->humanPlayer)->actionPointsPerTurn;
        PLAYER(game->aiPlayer)->actionPointsLeft =
            PLAYER(game->aiPlayer)->actionPointsPerTurn;
        PLAYER(game->humanPlayer)->turn ++;
        PLAYER(game->aiPlayer)->turn ++;


        // reset players
        thePlayerSystem.forEachECDo([] (Entity , PlayerComponent* pc) -> void {
            pc->actionPointsLeft = pc->actionPointsPerTurn;
            pc->turn++;
        });

        // reset soldier too
        theSoldierSystem.forEachECDo([] (Entity e, SoldierComponent* sc) -> void {
            sc->actionPointsLeft = sc->maxActionPointsPerTurn;
            SoldierSystem::UpdateUI(e, sc);
        });

        // update ui
        std::stringstream ss1;
        ss1 << "Turn: " << PLAYER(game->humanPlayer)->turn;
        TEXT(game->turn)->text = ss1.str();

        std::stringstream ss2;
        ss2 << "AP left: " << PLAYER(game->humanPlayer)->actionPointsLeft;
        TEXT(game->points)->text = ss2.str();

        return Scene::SelectCharacter;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateBeginTurnSceneHandler(PrototypeGame* game) {
        return new BeginTurnScene(game);
    }
}
