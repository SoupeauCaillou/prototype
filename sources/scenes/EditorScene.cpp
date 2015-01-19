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

#include "systems/GridSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../SacHelloWorldGame.h"
#include "../Level.h"

#include "base/SceneState.h"


class EditorScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;

    public:

    EditorScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("editor", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    void onPreEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreEnter(f);
    }

    void onEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onEnter(f);

        if (game->level) {
            game->grid = Level::load(game->gameThreadContext->assetAPI->loadAsset(game->level), false);
            game->grid->forEachCellDo([this] (const GridPos& pos) -> void {
                Entity e = game->grid->getEntitiesAt(pos).front();
                RENDERING(e)->color = typeToColor(GRID(e)->type);
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
        }
        AABB aabb = game->grid->boundingBox(false);
        TRANSFORM(game->camera)->position.x = (aabb.left + aabb.right) * 0.5f;
        TRANSFORM(game->camera)->position.y = (aabb. bottom + aabb.top) * 0.5f;
        glm::vec2 size = TRANSFORM(game->camera)->size;
        TRANSFORM(game->camera)->size.x = (aabb.right - aabb.left);
        TRANSFORM(game->camera)->size.y = TRANSFORM(game->camera)->size.x * size.y / size.x;
    }

    static Color typeToColor(bitfield_t b) {
        switch (b) {
            case Case::Empty: return Color(0.01, 1, 0.01);
            case Case::Dog:   return Color(0.9, 0.8, 0.03);
            case Case::Rock:  return Color(0.3, 0.3, 0.3);
            case Case::Start: return Color(0.9, 0.9, 0.9);
            case Case::End:   return Color(0.6, 0.1, 0.1);
            default:
                return Color(0, 0, 0);
        }
    }

    static char typeToChar(bitfield_t b) {
        switch (b) {
            case Case::Empty: return '.';
            case Case::Dog:   return 'D';
            case Case::Rock:  return 'X';
            case Case::Start: return 'S';
            case Case::End:   return 'E';
            default:
                return '.';
        }
    }

    void dumpLevel(Entity camera, HexSpatialGrid& grid) {
        LOGI("### Level Start");
        LOGI("11,9");
        SpatialGrid::Iterate::Result result =
            grid.iterate(invalidGridPos);

        std::stringstream s;
        while (result.valid) {
            auto& eList = game->grid->getEntitiesAt(result.pos);
            // LOGI(result.pos);
            for (auto e: eList) {
                auto* btn = theButtonSystem.Get(e, false);
                if (btn) {
                    s << typeToChar(GRID(e)->type);
                    break;
                }
            }
            result = grid.iterate(result.pos);

            if (result.newLine) {
                LOGI(s.str());
                s.str(""); s.clear();
            }
        }
        LOGI("### Level End\n");
    }

    Entity brushModeMaster = 0;

    Scene::Enum update(float) override {
        game->grid->forEachCellDo([this] (const GridPos& pos) -> void {
            auto& eList = game->grid->getEntitiesAt(pos);
            for (auto e: eList) {
                auto* btn = theButtonSystem.Get(e, false);
                if (btn) {

                    if (brushModeMaster == 0 || brushModeMaster == e) {
                        if (btn->clicked) {
                            LOGI(pos);
                            brushModeMaster = 0;
                            bitfield_t b = GRID(e)->type;

                            switch (b) {
                                case Case::Empty: b = Case::Rock; break;
                                case Case::Rock: b = Case::Start; break;
                                case Case::Start: b = Case::End; break;
                                case Case::End: b = Case::Dog; break;
                                case Case::Dog: b = Case::Empty; break;
                                default: b = Case::Empty; break;
                            }
                            GRID(e)->type = b;
                            RENDERING(e)->color = typeToColor(b);

                            dumpLevel(game->camera, *game->grid);
                        } else {
                            if (btn->mouseOver) {
                                LOGI_IF(brushModeMaster == 0, "Engaging brushModeMaster " << e);
                                brushModeMaster = e;
                            }
                        }
                        if (!theTouchInputManager.isTouched()) {
                            LOGI_IF(brushModeMaster, "Stopping brushModeMaster");
                            brushModeMaster = 0;
                        }
                    } else {
                        if (IntersectionUtil::pointRectangle(theTouchInputManager.getOverLastPosition(), TRANSFORM(e)->position, TRANSFORM(e)->size * BUTTON(e)->overSize, 0.0f)) {
                            GRID(e)->type = GRID(brushModeMaster)->type;
                            RENDERING(e)->color = typeToColor(GRID(e)->type);
                            dumpLevel(game->camera, *game->grid);
                        }
                    }
                }
            }
        });

        return Scene::Editor;
    }

    void onPreExit(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreExit(f);
    }

    void onExit(Scene::Enum to) override {
        SceneState<Scene::Enum>::onExit(to);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateEditorSceneHandler(SacHelloWorldGame* game) {
        return new EditorScene(game);
    }
}
