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

#include "base/EntityManager.h"
#include "systems/AnchorSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "PrototypeGame.h"
#include "util/Draw.h"
#include "util/Random.h"

#include <algorithm>

struct MazeGenerationScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;
    bool mazeBuilt;

    MazeGenerationScene(PrototypeGame* game)
        : SceneState<Scene::Enum>(
              "maze_generation", SceneEntityMode::DoNothing, SceneEntityMode::DoNothing) {
        this->game = game;
    }

    void setup(AssetAPI*) override {
        glm::vec2 first = PrototypeGame::firstCellPosition();

        for (int i=0; i<MAZE_SIZE; i++) {
            for (int j=0; j<MAZE_SIZE; j++) {
                game->grid[i][j].e = theEntityManager.CreateEntityFromTemplate("cell");
                TRANSFORM(game->grid[i][j].e)->position = first + glm::vec2(i, j);
            }
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        // cleanup walls
        for (auto e: game->walls) {
            if (e) {
                theEntityManager.DeleteEntity(e);
            }
        }
        game->walls.clear();
        game->walls.push_back(0); // dummy invalid wall

        // init cells

        for (int i=0; i<MAZE_SIZE; i++) {
            for (int j=0; j<MAZE_SIZE; j++) {
                game->grid[i][j].attr = cell_attibute::Out;

                for (int k=0; k<4; k++) {
                    // always create north and east walls
                    if ((k < 2) || (i==0 && k == 3) || (j==0 && k == 4)) {
                        Entity wall = theEntityManager.CreateEntityFromTemplate("wall");
                        ANCHOR(wall)->parent = game->grid[i][j].e;
                        ANCHOR(wall)->position = wallIdxToAnchor((direction::Enum)k);
                        ANCHOR(wall)->rotation = wallIdxToRotation((direction::Enum)k);
                        game->walls.push_back(wall);

                        game->grid[i][j].wallIndirect[k] = game->walls.size() - 1;
                    } else if (k == 2) {
                        // south
                        game->grid[i][j].wallIndirect[k] = game->grid[i][j-1].wallIndirect[0];
                    } else {
                        // west
                        game->grid[i][j].wallIndirect[k] = game->grid[i-1][j].wallIndirect[1];
                    }
                }
            }
        }

        initMaze();
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        for (int i=0; i<MAZE_SIZE; i++) {
            for (int j=0; j<MAZE_SIZE; j++) {
                RENDERING(game->grid[i][j].e)->color = attributeToColor(game->grid[i][j].attr);
            }
        }

        if (!buildMaze()) {
            return Scene::MazeGeneration;
        } else {
            return Scene::PlaceYourBets;
        }
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}


    typedef glm::ivec2 Coords;
    std::vector<Coords> in;
    std::vector<Coords> frontier;

    std::vector<Coords> neighbours(Coords c) {
        std::vector<Coords> arounds;
        if (c.y < (MAZE_SIZE - 1)) { arounds.push_back(Coords(c.x, c.y + 1)); }
        if (c.y >= 1 ) { arounds.push_back(Coords(c.x, c.y - 1)); }
        if (c.x < (MAZE_SIZE - 1)) { arounds.push_back(Coords(c.x + 1, c.y)); }
        if (c.x >= 1 ) { arounds.push_back(Coords(c.x - 1, c.y)); }
        return arounds;
    }

    void updateFrontier(Coords c) {
        std::vector<Coords> arounds = neighbours(c);

        for (size_t i=0; i<arounds.size(); i++) {
            const Coords& d = arounds[i];
            // not 'in' maze, nor in frontier
            if (std::find(in.begin(), in.end(), d) == in.end() &&
                std::find(frontier.begin(), frontier.end(), d) == frontier.end()) {
                frontier.push_back(d);

                game->grid[d.x][d.y].attr = cell_attibute::Frontier;
            }
        }
    }

    void initMaze() {
        glm::ivec2 first(Random::Int(0, MAZE_SIZE - 1), Random::Int(0, MAZE_SIZE - 1));
        game->grid[first.x][first.y].attr = cell_attibute::In;
        in.push_back(first);
        updateFrontier(first);
    }

    bool buildMaze() {
        if (frontier.empty()) {
            return true;
        }

        // pick a new cell
        int idx = Random::Int(0, frontier.size() - 1);
        const Coords c = frontier[idx];

        std::vector<Coords> n = neighbours(c);
        n.erase(std::remove_if(n.begin(), n.end(), [this] (const Coords& nb) {
            // remove if not in maze
            return std::find(in.begin(), in.end(), nb) == in.end();
        }), n.end());

        LOGF_IF(n.empty(), "Every frontier cell should have at least 1 neighbour in maze");
        Coords from = n[Random::Int(0, n.size() - 1)];

        in.push_back(c);
        game->grid[c.x][c.y].attr = cell_attibute::In;
        frontier.erase(frontier.begin() + idx);

        LOGF_IF(std::find(in.begin(), in.end(), from) == in.end(), "Uh");

        for (int i=0; i<4; i++) {
            int idx = game->grid[c.x][c.y].wallIndirect[i];
            if (game->walls[idx]) {
                RENDERING(game->walls[idx])->show = true;
            }
        }
        // update walls
        if (from.x < c.x) {
            int idx = game->grid[c.x][c.y].wallIndirect[direction::W];
            theEntityManager.DeleteEntity(game->walls[idx]);
            game->walls[idx] = 0;
            game->grid[c.x][c.y].wallIndirect[direction::W] = 0;
        }
        if (from.x > c.x) {
            int idx = game->grid[c.x][c.y].wallIndirect[direction::E];
            theEntityManager.DeleteEntity(game->walls[idx]);
            game->walls[idx] = 0;
            game->grid[c.x][c.y].wallIndirect[direction::E] = 0;
        }
        if (from.y < c.y) {
            int idx = game->grid[c.x][c.y].wallIndirect[direction::S];
            theEntityManager.DeleteEntity(game->walls[idx]);
            game->walls[idx] = 0;
            game->grid[c.x][c.y].wallIndirect[direction::S] = 0;
        }
        if (from.y > c.y)  {
            int idx = game->grid[c.x][c.y].wallIndirect[direction::N];
            theEntityManager.DeleteEntity(game->walls[idx]);
            game->walls[idx] = 0;
            game->grid[c.x][c.y].wallIndirect[direction::N] = 0;
        }

        Draw::Vec2(
            TRANSFORM(game->grid[from.x][from.y].e)->position,
            TRANSFORM(game->grid[c.x][c.y].e)->position - TRANSFORM(game->grid[from.x][from.y].e)->position,
            Color(0, 0, 1));

        updateFrontier(c);

        return false;
    }


    Color attributeToColor(cell_attibute::Enum attr) {
        switch(attr) {
            case cell_attibute::In: return Color(1, 1, 1);
            case cell_attibute::Frontier: return Color(0.9, 0.9, 0.9);
            case cell_attibute::Out:
            default:
                return Color(0.8, 0.8, 0.8);
        }
    }



    glm::vec2 wallIdxToAnchor(direction::Enum dir) {
        switch (dir) {
            case direction::S: return glm::vec2(0, -0.5f);
            case direction::N: return glm::vec2(0, 0.5f);
            case direction::E: return glm::vec2(0.5f, 0);
            case direction::W: return glm::vec2(-0.5f, 0);
            default: return glm::vec2(0.0f);
        }
    }
    float wallIdxToRotation(direction::Enum dir) {
        switch (dir) {
            case direction::N:
            case direction::S: return glm::pi<float>() * 0.5f;
            case direction::E:
            case direction::W:
            default:           return 0.0f;
        }
    }

};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMazeGenerationSceneHandler(PrototypeGame* game) {
        return new MazeGenerationScene(game);
    }
}
