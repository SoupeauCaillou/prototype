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
#include "systems/MessageSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"
#include "api/NetworkAPI.h"

#include "PrototypeGame.h"

#include "systems/PlayerSystem.h"
#include "systems/SoldierSystem.h"
#include "systems/FlickSystem.h"
#include "systems/KnightSystem.h"
#include "systems/ArcherSystem.h"
#include "systems/ProjectileSystem.h"
#include "systems/MessageSystem.h"

struct InGameScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    InGameScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        theSoldierSystem.forEachECDo([this] (Entity e, SoldierComponent* sc) -> void {
            const auto* fc = theFlickSystem.Get(e, false);
            if (sc->player == game->player) {
                if (!fc) {
                    ADD_COMPONENT(e, Flick);
                    FLICK(e)->maxForce = 750;
                    FLICK(e)->activationDistance = Interval<float>(0.2, 3);
                    FLICK(e)->enabled = true;
                }
            } else {
                if (fc) {
                 //   theEntityManager.RemoveComponent(e, &theFlickSystem);
                }
            }
        });
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        theFlickSystem.Update(dt);

        if (game->gameThreadContext->networkAPI->getStatus() != NetworkStatus::ConnectedToServer) {
            auto& mpc = theMessageSystem.getAllComponents();
            for (auto it=mpc.begin(); it!=mpc.end(); ) {
                if (it->second->type == MessageType::Flick) {
                    Entity e = it->second->flick.target;
                    FLICK(e)->flickingStartedAt = TRANSFORM(e)->position;
                    FLICK(e)->status = FlickStatus::Moving;
                    PHYSICS(e)->addForce(it->second->flick.force, glm::vec2(0.0f), 0.016);
                    Entity m = it->first;
                    ++it;
                    theEntityManager.DeleteEntity(m);
                } else {
                    ++it;
                }
            }

            thePlayerSystem.Update(dt);
            theSoldierSystem.Update(dt);
            theKnightSystem.Update(dt);
            theArcherSystem.Update(dt);
            theProjectileSystem.Update(dt);
        }

        return Scene::InGame;
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
    StateHandler<Scene::Enum>* CreateInGameSceneHandler(PrototypeGame* game) {
        return new InGameScene(game);
    }
}
