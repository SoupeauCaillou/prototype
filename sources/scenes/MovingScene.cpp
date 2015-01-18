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
#include "Scenes.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"

#include "systems/ADSRSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/MoveCommandSystem.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../SacHelloWorldGame.h"

#include "base/SceneState.h"


class MovingScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;

    public:

    MovingScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("moving", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    void onPreEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreEnter(f);
    }

    void onEnter(Scene::Enum f) {
        SceneState<Scene::Enum>::onEnter(f);
        theMoveCommandSystem.forEachEntityDo([] (Entity e) -> void {
            ADSR(e)->active = true;
        });
    }

    Scene::Enum update(float) {
        int movementsLeft = theMoveCommandSystem.entityCount();
        if (movementsLeft == 0)
            return Scene::Game;
        std::vector<Entity> toDelete;
        theMoveCommandSystem.forEachECDo([this, &toDelete] (Entity e, MoveCommandComponent* mc) -> void {
            float progress = ADSR(e)->value;
            const glm::vec2 from = game->grid->gridPosToPosition(mc->from);
            const glm::vec2 to = game->grid->gridPosToPosition(mc->to);
            if (progress >= 1) {
                toDelete.push_back(e);
                game->grid->removeEntityFrom(mc->target, mc->from);
                game->grid->addEntityAt(mc->target, mc->to, true);
            } else {
                glm::vec2 direction = (to - from);
                TRANSFORM(mc->target)->position = from + direction * progress;
                TRANSFORM(mc->target)->rotation = glm::atan2<float, glm::mediump>(direction.y, direction.x);
            }
        });
        for (auto e: toDelete) {
            theEntityManager.DeleteEntity(e);
        }
        return Scene::Moving;
    }

    void onPreExit(Scene::Enum f) {
        SceneState<Scene::Enum>::onPreExit(f);
    }

    void onExit(Scene::Enum to) {
        SceneState<Scene::Enum>::onExit(to);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMovingSceneHandler(SacHelloWorldGame* game) {
        return new MovingScene(game);
    }
}
