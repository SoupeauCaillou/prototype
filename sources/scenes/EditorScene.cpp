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

#include "systems/GridSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"
#include "../SacHelloWorldGame.h"

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

    void onEnter(Scene::Enum f) {
        SceneState<Scene::Enum>::onEnter(f);
    }

    static Color typeToColor(bitfield_t b) {
        switch (b) {
            case Case::Empty: return Color(0.01, 1, 0.01);
            case Case::Rock:  return Color(0.3, 0.3, 0.3);
            case Case::Start: return Color(0.9, 0.9, 0.9);
            case Case::End:   return Color(0.1, 0.1, 0.1);
            default:
                return Color(0, 0, 0);
        }
    }

    static char typeToChar(bitfield_t b) {
        switch (b) {
            case Case::Empty: return 'O';
            case Case::Rock:  return 'X';
            case Case::Start: return 'S';
            case Case::End:   return 'E';
            default:
                return '0';
        }
    }

    void dumpLevel(Entity camera, HexSpatialGrid& grid) {
        auto topLeft = TRANSFORM(camera)->size * glm::vec2(-0.5, 0.5);

        GridPos pos = grid.positionToGridPos(topLeft);
        LOGI(pos << '\n');
        GridPos previousPos = pos;
        for (;grid.isPosValid(pos);) {
            std::stringstream s;
            for (;; pos.q += 1) {
                if (grid.isPosValid(pos)) {
                    auto& eList = game->grid.getEntitiesAt(pos);
                    for (auto e: eList) {
                        auto* btn = theButtonSystem.Get(e, false);
                        if (btn) {
                            s << typeToChar(GRID(e)->type) << ',';
                            break;
                        }
                    }
                } else {
                    break;
                }
            }
            pos = previousPos;
            // try bottomleft
            pos.r -= 1;
            if (! grid.isPosValid(pos) ) {
                // upperright
                pos = previousPos;
                pos.q += 1;pos.r -= 1;
            }
            previousPos = pos;
            LOGI(s.str());
        }

    }

    Scene::Enum update(float) {
        game->grid.forEachCellDo([this] (const GridPos& pos) -> void {
            auto& eList = game->grid.getEntitiesAt(pos);
            for (auto e: eList) {
                auto* btn = theButtonSystem.Get(e, false);
                if (btn && btn->clicked) {
                    bitfield_t b = GRID(e)->type;
                    LOGI(pos);

                    switch (b) {
                        case Case::Empty: b = Case::Rock; break;
                        case Case::Rock: b = Case::Start; break;
                        case Case::Start: b = Case::End; break;
                        case Case::End: b = Case::Empty; break;
                        default: b = Case::Empty; break;
                    }
                    GRID(e)->type = b;
                    RENDERING(e)->color = typeToColor(b);

                    dumpLevel(game->camera, game->grid);
                }
            }
        });

        return Scene::Editor;
    }

    void onPreExit(Scene::Enum f) {
        SceneState<Scene::Enum>::onPreExit(f);
    }

    void onExit(Scene::Enum to) {
        SceneState<Scene::Enum>::onExit(to);
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateEditorSceneHandler(SacHelloWorldGame* game) {
        return new EditorScene(game);
    }
}
