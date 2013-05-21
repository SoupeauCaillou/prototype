/*
    This file is part of RecursiveRunner.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

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


#include "ParatroopersGame.h"

#include "systems/AISystem.h"
#include "systems/DCASystem.h"
#include "systems/InputSystem.h"
#include "systems/ParachuteSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/PlaneSystem.h"
#include "systems/PlayerSystem.h"

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"
#include "base/StateMachine.h"
#include "base/TouchInputManager.h"

#include "systems/AnchorSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/AnimationSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/NetworkSystem.h"

#include "api/KeyboardInputHandlerAPI.h"

#include "api/linux/NetworkAPILinuxImpl.h"
#include "util/IntersectionUtil.h"

#include <glm/gtx/compatibility.hpp>
#include "glm/gtx/norm.hpp"
#include <glm/gtx/rotate_vector.hpp>

#include "Scenes.h"


#include <sstream>
#include <vector>
#include <iomanip>

struct TestScene : public StateHandler<Scene::Enum> {
    ParatroopersGame* game;

    Entity scores[2];

    TestScene(ParatroopersGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() override {
        if (game->networkMode)
            game->gameThreadContext->networkAPI->connectToLobby(game->networkNickname, game->lobbyAddress.c_str());
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    bool updatePreEnter(Scene::Enum, float) override {
        if (game->networkMode)
            return game->gameThreadContext->networkAPI->isConnectedToAnotherPlayer();
        else
            return true;
    }

    void onEnter(Scene::Enum) override {
        // purely local entities first
        scores[0] = theEntityManager.CreateEntity("score_g",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("score1"));
        scores[1] = theEntityManager.CreateEntity("score_b",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("score2"));

        // then, gameplay (shared) entities
        if (game->networkMode) {
            if (!game->gameThreadContext->networkAPI->amIGameMaster())
                return;
        }

        for (int i=0; i<2; i++) {
            #define __(b) (std::string(b) + (i ? "2" : "1"))

            Entity player = theEntityManager.CreateEntity(__("player"),
                EntityType::Persistent,
                theEntityManager.entityTemplateLibrary.load(__("player")));

            Entity plane = theEntityManager.CreateEntity(__("plane"),
                EntityType::Persistent,
                theEntityManager.entityTemplateLibrary.load(__("plane")));

            Entity dca = theEntityManager.CreateEntity(__("dca"),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(__("DCA")));

            PLANE(plane)->owner = player;
            DCA(dca)->owner = player;

            if (game->networkMode && i == 1) {
                // remove AI from player2
                theAISystem.Delete(player);
                NETWORK(player)->newOwnerShipRequest = 1;
            }
            #undef __
        }
    }

    void updateInput(Entity entity, InputComponent* ic) {
        const glm::vec2& touchPos = theTouchInputManager.getTouchLastPosition(0);

        // touched a plane -> spawn new paratrooper
        FOR_EACH_ENTITY_COMPONENT(Plane, plane, planeC)
#if SAC_MOBILE
            if (IntersectionUtil::pointRectangle(touchPos,
                    TRANSFORM(plane)->position,
                    TRANSFORM(plane)->size)) {
#else
            if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_a)) {
#endif
                if (planeC->owner == entity) {
                    ic->action = Action::Spawn;
                    ic->SpawnParams.plane = plane;
                    return;
                } else {
                    LOGI("Clicked on a plane that is not mine: " << planeC->owner);
                }
            }
        }

        // touched a guy -> open parachute
        FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, paraC)
            if (!paraC->dead &&
                !paraC->parachute) {

                if (IntersectionUtil::pointRectangle(touchPos,
                    TRANSFORM(para)->position,
                    TRANSFORM(para)->size)) {

                    if (paraC->owner == entity) {
                        ic->action = Action::OpenParachute;
                        ic->OpenParachuteParams.paratrooper = para;
                        return;
                    } else {
                        LOGI("Tried to open a parachute of the other team: " << paraC->owner);
                    }
                }
            }
        }


        // touched in DCA area -> fire
        FOR_EACH_ENTITY_COMPONENT(DCA, dca, dcaC)
            if (glm::distance2(touchPos, TRANSFORM(dca)->position) <= glm::pow(dcaC->maximalDistanceForActivation, 2.0f)) {

                if (dcaC->owner == entity) {
                    ic->action = Action::Fire;
                    ic->FireParams.aim = touchPos;
                    ic->FireParams.dca = dca;
                    return;
                } else {
                    LOGI("Tried to use other team's DCA: " << dcaC->owner);
                    PLAYER(entity)->score ++;
                }
            }
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
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
            return Scene::Test;

        LOGI_EVERY_N(100, "Found my player. Entity: " << myPlayer);

#if SAC_MOBILE
        bool touching = theTouchInputManager.isTouched(0);
#else
        bool touching = theTouchInputManager.isTouched(0) || game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_a);
#endif

        InputComponent* ic = INPUT(myPlayer);
        ic->action = Action::None;

        if (touching) {
            updateInput(myPlayer, ic);
        }

        // debug
        if (theTouchInputManager.isTouched(1)) {
            FOR_EACH_ENTITY(Paratrooper, p)
                static float lastTouch = 0;
                float ti = TimeUtil::GetTime();
                if (ti - lastTouch > 1) {
                    glm::vec2 cursorPosition = theTouchInputManager.getTouchLastPosition(1);
                    if (IntersectionUtil::pointRectangle(cursorPosition, TRANSFORM(p)->position, TRANSFORM(p)->size)) {
                        LOGW("Soldier '" << theEntityManager.entityName(p) << p << "' is dead");
                        PARATROOPER(p)->dead = true;
                    }
                    if (PARATROOPER(p)->parachute) {
                        Entity parachute = PARATROOPER(p)->parachute;
                        if (IntersectionUtil::pointRectangle(cursorPosition, TRANSFORM(parachute)->position, TRANSFORM(parachute)->size)) {
                            LOGI("adding damage for " << theEntityManager.entityName(parachute) << parachute
                                << "pos: " << TRANSFORM(p)->position << " size:" << TRANSFORM(parachute)->size
                                << "cursor: " << cursorPosition
                                << " at " << cursorPosition - TRANSFORM(parachute)->position + TRANSFORM(parachute)->position / 2.f);
                            PARACHUTE(parachute)->damages.push_back(cursorPosition - TRANSFORM(parachute)->position + TRANSFORM(parachute)->size / 2.f);
                        }

                        Entity hole = theEntityManager.CreateEntity("hole", EntityType::Volatile,
                             theEntityManager.entityTemplateLibrary.load("hole"));
                        PARACHUTE(parachute)->holes.push_back(hole);
                        ANCHOR(hole)->parent = parachute;
                        ANCHOR(hole)->position = glm::rotate(cursorPosition - TRANSFORM(parachute)->position,
                            -TRANSFORM(parachute)->rotation);
                    }
                    lastTouch = ti;
                }
            }
        }

        // update score
        LOGW_IF(2 != players.size(), "Scores count: " << 2 <<
            "different from players count : " << players.size() << ".");
        for (unsigned i=0; i<players.size() && i<2; i++) {
            std::stringstream ss;
            ss << "Score: " << PLAYER(players[i])->score;
            TEXT_RENDERING(scores[i])->text = ss.str();
        }

        return Scene::Test;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onExit(Scene::Enum) override {
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateTestSceneHandler(ParatroopersGame* game) {
        return new TestScene(game);
    }
}
