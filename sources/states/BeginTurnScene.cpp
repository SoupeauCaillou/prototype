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
#include "PrototypeGame.h"
#include "systems/TextRenderingSystem.h"

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
    void onPreEnter(Scene::Enum) {}
    bool updatePreEnter(Scene::Enum, float) override {return true;}
    void onEnter(Scene::Enum) override {
        TEXT_RENDERING(game->banner)->color = Color(0, 1, 0);
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

        std::stringstream ss1;
        ss1 << "Turn: " << PLAYER(game->humanPlayer)->turn;
        TEXT_RENDERING(game->turn)->text = ss1.str();

        std::stringstream ss2;
        ss2 << "AP left: " << PLAYER(game->humanPlayer)->actionPointsLeft;
        TEXT_RENDERING(game->points)->text = ss2.str();

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
