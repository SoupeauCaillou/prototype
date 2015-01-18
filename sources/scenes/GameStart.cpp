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

    void onEnter(Scene::Enum f) {
        SceneState<Scene::Enum>::onEnter(f);

        if (game->level) {
            game->grid = Level::load(game->gameThreadContext->assetAPI->loadAsset(game->level));
            theGridSystem.forEachECDo([this] (Entity e, GridComponent* gc) -> void {
                if (gc->type == Case::Dog) {
                    LOGI("dog found: " << e);
                    dog = e;
                }
            });
        }
        if (!game->grid) {
            LOGE_IF(game->level, "Invalid level filename '" << game->level << "'");

            game->grid = new HexSpatialGrid(11, 9, 2.6);
            game->grid->forEachCellDo([this] (const GridPos& pos) -> void {
                std::string type = std::string("field/cell_grass");
                Entity e = theEntityManager.CreateEntityFromTemplate(type.c_str());
                game->grid->addEntityAt(e, pos, true);
            });
            dog = theEntityManager.CreateEntityFromTemplate("dog");
            game->grid->addEntityAt(dog, GridPos(0, 0), true);
        }

        AABB aabb = game->grid->boundingBox(false);
        TRANSFORM(game->camera)->position.x = (aabb.left + aabb.right) * 0.5f;
        TRANSFORM(game->camera)->position.y = (aabb. bottom + aabb.top) * 0.5f;
        glm::vec2 size = TRANSFORM(game->camera)->size;
        TRANSFORM(game->camera)->size.x = (aabb.right - aabb.left);
        TRANSFORM(game->camera)->size.y = TRANSFORM(game->camera)->size.x * size.y / size.x;
    }

    Scene::Enum update(float) {
        return Scene::Game;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateGameStartSceneHandler(SacHelloWorldGame* game) {
        return new GameStartScene(game);
    }
}
