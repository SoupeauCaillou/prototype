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
#include "systems/AnchorSystem.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../HerdingDogGame.h"
#include "../Level.h"

#include "base/SceneState.h"


class EditorScene : public SceneState<Scene::Enum> {
    HerdingDogGame* game;


    std::map<GridPos, Entity> logos;
    public:

    EditorScene(HerdingDogGame* game) : SceneState<Scene::Enum>("editor", SceneEntityMode::DoNothing, SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void onPreEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreEnter(f);
    }

    void onEnter(Scene::Enum f) override {
        SceneState<Scene::Enum>::onEnter(f);

        delete game->grid;
        game->grid=0;

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

        game->grid->forEachCellDo([this] (const GridPos& pos) -> void {
            Entity e = game->grid->getEntitiesAt(pos).front();
            Entity logo = theEntityManager.CreateEntityFromTemplate("editor/editor_logo");
            ANCHOR(logo)->parent = e;
            RENDERING(logo)->texture = typeToTexture(GRID(game->grid->getEntitiesAt(pos).front())->type);
            logos.insert(std::make_pair(pos, logo));
        });

        AABB aabb = game->grid->boundingBox(false);
        TRANSFORM(game->camera)->position.x = (aabb.left + aabb.right) * 0.5f;
        TRANSFORM(game->camera)->position.y = (aabb. bottom + aabb.top) * 0.5f;
        glm::vec2 size = TRANSFORM(game->camera)->size;
        TRANSFORM(game->camera)->size.x = (aabb.right - aabb.left);
        TRANSFORM(game->camera)->size.y = TRANSFORM(game->camera)->size.x * size.y / size.x;
        if (TRANSFORM(game->camera)->size.y < (aabb.top - aabb.bottom)) {
            TRANSFORM(game->camera)->size.y = (aabb.top - aabb.bottom);
            TRANSFORM(game->camera)->size.x = TRANSFORM(game->camera)->size.y * size.x / size.y;
        }

    }

    static Color typeToColor(bitfield_t b) {
        switch (b) {
            case Case::Empty:  return Color(0.01, 1, 0.01);
            case Case::Dog:    return Color(0.9, 0.8, 0.03);
            case Case::Rock:   return Color(0.3, 0.3, 0.3);
            case Case::Start:  return Color(0.9, 0.9, 0.9);
            case Case::End:    return Color(0.6, 0.1, 0.1);
            case Case::Flower: return Color(1.0, 0.2, 0.9);
            default:
                return Color(0, 0, 0);
        }
    }

    static hash_t typeToTexture(bitfield_t b) {
        switch (b) {
            case Case::Empty:  return 0;
            case Case::Dog:    return HASH("dog_logo", 0x0);
            case Case::Rock:   return 0;
            case Case::Start:  return HASH("sheep_logo", 0x0);
            case Case::End:    return HASH("fin_logo", 0x0);
            case Case::Flower: return 0;
            default:
                return 0;
        }
    }

    static char typeToChar(bitfield_t b) {
        switch (b) {
            case Case::Empty:  return '.';
            case Case::Dog:    return 'D';
            case Case::Rock:   return 'X';
            case Case::Start:  return 'S';
            case Case::End:    return 'E';
            case Case::Flower: return 'F';
            default:
                return '.';
        }
    }

    void dumpLevel(Entity camera, HexSpatialGrid& grid) {
        LOGI("### Level Start");
        SpatialGrid::Iterate::Result result =
            grid.iterate(invalidGridPos);

        std::stringstream output;
        output << '\n';
        output << grid.getWidth() << ',' << grid.getHeight() << '\n';
        while (result.valid) {
            auto& eList = game->grid->getEntitiesAt(result.pos);
            // LOGI(result.pos);
            for (auto e: eList) {
                auto* btn = theButtonSystem.Get(e, false);
                if (btn) {
                    output << typeToChar(GRID(e)->type);
                    break;
                }
            }
            result = grid.iterate(result.pos);

            if (result.newLine) {
                output << '\n';
            }
        }
        LOGI(output.str());
        LOGI("### Level End\n");
    }

    Entity brushModeMaster = 0;

    Scene::Enum update(float) override {
        game->grid->forEachCellDo([this] (const GridPos& pos) -> void {
            Entity logo = logos[pos];

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
                                case Case::Empty:   b = Case::Rock; break;
                                case Case::Rock:    b = Case::Start; break;
                                case Case::Start:   b = Case::End; break;
                                case Case::End:     b = Case::Dog; break;
                                case Case::Dog:     b = Case::Flower; break;
                                case Case::Flower:  b = Case::Empty; break;
                                default:            b = Case::Empty; break;
                            }
                            GRID(e)->type = b;
                            RENDERING(e)->color = typeToColor(b);
                            RENDERING(logo)->texture = typeToTexture(b);

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
                            RENDERING(logo)->texture = typeToTexture(GRID(e)->type);
                            dumpLevel(game->camera, *game->grid);
                        }
                    }
                }
            }

            RENDERING(logo)->show = RENDERING(logo)->texture != 0;
        });

        return Scene::Editor;
    }

    void onPreExit(Scene::Enum f) override {
        SceneState<Scene::Enum>::onPreExit(f);

        for (auto logo : logos) {
            theEntityManager.DeleteEntity(logo.second);
        }
        logos.clear();
    }

    void onExit(Scene::Enum to) override {
        SceneState<Scene::Enum>::onExit(to);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateEditorSceneHandler(HerdingDogGame* game) {
        return new EditorScene(game);
    }
}
