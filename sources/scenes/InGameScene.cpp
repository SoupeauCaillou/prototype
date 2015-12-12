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

#include "base/SceneState.h"

#include "PrototypeGame.h"
#include "systems/ZSQDSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "util/IntersectionUtil.h"

#include "api/KeyboardInputHandlerAPI.h"
#include <SDL2/SDL.h>

struct InGameScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;

    InGameScene(PrototypeGame* game)
        : SceneState<Scene::Enum>("in_game",
                                  SceneEntityMode::DoNothing,
                                  SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void setup(AssetAPI*) override {}

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        RENDERING(game->guy[0])->show = true;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        /* position */
        previousDirection = ZSQD(game->guy[0])->currentDirection;
        fixPosition(game);
        previousPosition = TRANSFORM(game->guy[0])->position;

        // Draw::Rectangle(TRANSFORM(grid[x][y].e)->position, glm::vec2(1, 1), 0.0f, Color(1, 0, 1));

        if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_LEFT)) {
            ZSQD(game->guy[0])->directions.push_back(glm::vec2(-1.0f, 0.0));
        } else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_RIGHT)) {
            ZSQD(game->guy[0])->directions.push_back(glm::vec2(1.0f, 0.0));
        }
        if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_UP)) {
            ZSQD(game->guy[0])->directions.push_back(glm::vec2(0.0f, 1.0));
        } else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_DOWN)) {
            ZSQD(game->guy[0])->directions.push_back(glm::vec2(0.0f, -1.0));
        }
        return Scene::InGame;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}



    int iterBeginValue(int current, float dir) {
        return (dir >= 0 || current == 0) ? current : current - 1;
    }
    int iterEndValue(int current, float dir) {
        return (dir <= 0 || current == (MAZE_SIZE - 1)) ? current : current + 1;
    }

    glm::vec2 previousDirection, previousPosition;
    bool tryPosition(PrototypeGame* game, const glm::vec2& testPosition, int _x, int _y) {
        auto* tc = TRANSFORM(game->guy[0]);
        const glm::vec2 old = tc->position;
        tc->position = testPosition;

        int xStart = iterBeginValue(_x, previousDirection.x);
        int xEnd = iterEndValue(_x, previousDirection.x);

        int yStart = iterBeginValue(_y, previousDirection.y);
        int yEnd = iterEndValue(_y, previousDirection.y);

        bool success = true;
        for (int x=xStart; x<=xEnd; x++) {
            for (int y=yStart; y<=yEnd; y++) {
                const Cell& cell = game->grid[x][y];
                for (int i=0; i<4; i++) {
                    Entity wall = game->walls[cell.wallIndirect[i]];
                    if (wall) {
                        if (IntersectionUtil::rectangleRectangleAABB(
                            tc,
                            TRANSFORM(wall))) {
                            success = false;
                            goto end;
                        }
                    }
                }
            }
        }
    end:
        tc->position = old;
        return success;
    }

    void fixPosition(PrototypeGame* game) {
        glm::vec2 first = PrototypeGame::firstCellPosition();
        glm::vec2 p = TRANSFORM(game->guy[0])->position - first;
        int x = glm::round(p.x);
        int y = glm::round(p.y);

        const auto newPosition = TRANSFORM(game->guy[0])->position;

        if (tryPosition(game, newPosition, x, y)) {
            return;
        }

        if (previousDirection.x) {
            glm::vec2 cancelXMovement(previousPosition.x, newPosition.y);

            if (tryPosition(game, cancelXMovement, x, y)) {
                TRANSFORM(game->guy[0])->position = cancelXMovement;
                return;
            }
        }

        if (previousDirection.y) {
            glm::vec2 cancelYMovement(newPosition.x, previousPosition.y);

            if (tryPosition(game, cancelYMovement, x, y)) {
                TRANSFORM(game->guy[0])->position = cancelYMovement;
                return;
            }
        }

        TRANSFORM(game->guy[0])->position = previousPosition;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateInGameSceneHandler(PrototypeGame* game) {
        return new InGameScene(game);
    }
}
