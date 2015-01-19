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
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"

#include "systems/ADSRSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextSystem.h"
#include "systems/MusicSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/GridSystem.h"
#include "systems/RenderingSystem.h"

#include "util/Draw.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../SacHelloWorldGame.h"

#include "../Level.h"
#include "base/SceneState.h"


class GameStartScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;
    Entity dog;
    // Which GridPos are occupied for next turn, and by whom.
    std::vector<std::pair<GridPos, Entity>> unavailable;

    public:

    GameStartScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("game_start", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    void onPreEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreEnter(f);
    }

    void onEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onEnter(f);

        if (!game->level) {
            game->level = "1.lvl";
        }

        game->grid = Level::load(game->gameThreadContext->assetAPI->loadAsset(game->level), true);
        theGridSystem.forEachECDo([this] (Entity e, GridComponent* gc) -> void {
            if (gc->type == Case::Dog) {
                LOGI("dog found: " << e);
                dog = e;
            }
        });

        AABB aabb = game->grid->boundingBox(false);
        TRANSFORM(game->camera)->position.x = (aabb.left + aabb.right) * 0.5f;
        TRANSFORM(game->camera)->position.y = (aabb. bottom + aabb.top) * 0.5f;
        glm::vec2 size = TRANSFORM(game->camera)->size;
        TRANSFORM(game->camera)->size.x = (aabb.right - aabb.left);
        TRANSFORM(game->camera)->size.y = TRANSFORM(game->camera)->size.x * size.y / size.x;

        game->updateMovesCount(0);
    }

    Scene::Enum update(float) override {
        TEXT(game->movesCountE)->show = true;
        return Scene::Game;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameStartSceneHandler(SacHelloWorldGame* game) {
        return new GameStartScene(game);
    }
}
