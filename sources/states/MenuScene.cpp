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

#include "api/NetworkAPI.h"

#include "base/EntityManager.h"
#include "base/TouchInputManager.h"

#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/ActionSystem.h"
#include "systems/MorpionGridSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/PlayerSystem.h"

#include "PrototypeGame.h"

struct MenuScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity startBtn, networkTestBtn;

    MenuScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        if (game->networkMode) {
            game->gameThreadContext->networkAPI->connectToLobby(game->networkNickname, game->lobbyAddress.c_str());
        }
        startBtn = theEntityManager.CreateEntity("startBtn",
            EntityType::Volatile, theEntityManager.entityTemplateLibrary.load("button"));
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        RENDERING(startBtn)->show =
        BUTTON(startBtn)->enabled = true;

        // then, gameplay (shared) entities
        if (game->networkMode) {
            if (!game->gameThreadContext->networkAPI->amIGameMaster()) {
                LOGI("I am not the master :(.... " << game->networkNickname);
                return;
            }
        }
        LOGI("I am the master! " << game->networkNickname);

        for (int i=0; i<2; i++) {
            #define __(b) (std::string(b) + (i ? "2" : "1"))

            Entity player = theEntityManager.CreateEntity(__("player"),
                EntityType::Persistent,
                theEntityManager.entityTemplateLibrary.load(__("player")));

            //yup, it's a bit uggly..
            if (i == 1) {
                if (game->networkMode) {
                    NETWORK(player)->newOwnerShipRequest = 1;
                }
                game->player1 = player;
            } else {
                game->player2 = player;
            }
            #undef __
        }

        networkTestBtn = theEntityManager.CreateEntity("testBtn",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("button_testnetwork"));

        for (int i = 0; i < 81; ++i) {
            std::stringstream name;
            name << "grid_cell" << i / 9 << "/" << i % 9;
            game->grid[i] = theEntityManager.CreateEntity(name.str(),
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_cell"));
            MORPION_GRID(game->grid[i])->i = i / 9;
            MORPION_GRID(game->grid[i])->j = i % 9;
            TRANSFORM(game->grid[i])->position = theMorpionGridSystem.gridCellToPosition(i / 9, i % 9);
        }
    }

    bool updatePreEnter(Scene::Enum, float) override {
        if (game->networkMode)
            return game->gameThreadContext->networkAPI->isConnectedToAnotherPlayer();
        else
            return true;
    }
    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    void updateInput(Entity entity, ActionComponent* ac) {
        if (! networkTestBtn) {
            for (auto bc : theButtonSystem.getAllComponents()) {
                if (theEntityManager.entityName(bc.first) == "testBtn") {
                    networkTestBtn = bc.first;
                }
            }
        } else {
            if (BUTTON(networkTestBtn)->clicked) {
                ac->action = EAction::ClickButton;
                ac->ClickButtonParams.button = networkTestBtn;
                ac->ClickButtonParams.color = Color::random();
            }
        }
    }

    Scene::Enum update(float) override {
        // Retrieve all players
        std::vector<Entity> players = thePlayerSystem.RetrieveAllEntityWithComponent();
        Entity myPlayer = 0;

        // Pick mine
        bool gameMaster = (!game->networkMode || game->gameThreadContext->networkAPI->amIGameMaster());

        for_each(players.begin(), players.end(), [&myPlayer, gameMaster] (Entity e) -> void {
            if (PLAYER(e)->id == (gameMaster ? 0 : 1)) myPlayer = e;
        });
        LOGW_IF(myPlayer == 0, "Cannot find my player :'( (" << players.size());
        if (myPlayer == 0)
            return Scene::Menu;

        LOGI_EVERY_N(100, "Found my player. Entity: " << myPlayer);

        ActionComponent* ac = ACTION(myPlayer);
        ac->action = EAction::None;

        updateInput(myPlayer, ac);

        if (BUTTON(startBtn)->clicked) {
            return Scene::GameStart;
        }


        return Scene::Menu;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
        RENDERING(startBtn)->show =
        BUTTON(startBtn)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game) {
        return new MenuScene(game);
    }
}
