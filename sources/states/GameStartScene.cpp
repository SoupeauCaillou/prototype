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
#include "PlayerSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "base/TouchInputManager.h"
#include "PrototypeGame.h"

struct GameStartScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    GameStartScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    std::vector<Entity> players;
    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        // create my player
        game->myPlayer = theEntityManager.CreateEntityFromTemplate("player");
        PLAYER(game->myPlayer)->name = game->nickName;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        unsigned count = 0;
        thePlayerSystem.forEachECDo([this, &count] (Entity, PlayerComponent* pc) -> void {
            if (players.size() <= count) {
                players.push_back(theEntityManager.CreateEntityFromTemplate("menu/net_player"));
                TEXT(players.back())->show = RENDERING(players.back())->show = true;
            }

            std::stringstream ss;
            ss << pc->name << ": " << (pc->ready ? "ready" : "not");
            TEXT(players[count])->text = ss.str();
            RENDERING(players[count])->color = pc->ready ? Color(0, 1, 0) : Color(1, 0, 0);
            count++;
        });

        for (unsigned i=1; i<players.size(); i++) {
            TRANSFORM(players[i])->position = TRANSFORM(players[i-1])->position + TRANSFORM(players[i-1])->size * glm::vec2(1.1f, .0f);
        }

        if (theTouchInputManager.hasClicked()) {
            PLAYER(game->myPlayer)->ready = !PLAYER(game->myPlayer)->ready;
        }

        return Scene::GameStart;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
        for (auto p: players)
            theEntityManager.DeleteEntity(p);
    }

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameStartSceneHandler(PrototypeGame* game) {
        return new GameStartScene(game);
    }
}
