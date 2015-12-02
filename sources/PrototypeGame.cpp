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

#include "PrototypeGame.h"

#include "base/PlacementHelper.h"
#include "base/StateMachine.inl"
#include "base/EntityManager.h"

#include "systems/AnchorSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/ZSQDSystem.h"
#include "base/TimeUtil.h"
#include "util/Random.h"

#include <algorithm>

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include "api/NetworkAPI.h"
#if SAC_LINUX && SAC_DESKTOP
#include <unistd.h>
#endif

#include "api/KeyboardInputHandlerAPI.h"
#include <SDL2/SDL.h>

PrototypeGame::PrototypeGame() : Game() {
    registerScenes(this, sceneStateMachine);
}

namespace cell_attibute
{
    enum Enum { In, Frontier, Out };
}
Color attributeToColor(cell_attibute::Enum attr) {
    switch(attr) {
        case cell_attibute::In: return Color(1, 1, 1);
        case cell_attibute::Frontier: return Color(0.9, 0.3, 0.3);
        case cell_attibute::Out:
        default:
            return Color(0.6, 0.6, 0.6);
    }
}

const int MAZE_SIZE = 12;
struct Cell {
    Entity e;
    cell_attibute::Enum attr;

    int wallIndirect[4];
};
Cell grid[MAZE_SIZE][MAZE_SIZE];

namespace direction
{
    enum Enum { N = 0, E, S, W };
}

glm::vec2 wallIdxToAnchor(direction::Enum dir) {
    switch (dir) {
        case direction::S: return glm::vec2(0, -0.5f);
        case direction::N: return glm::vec2(0, 0.5f);
        case direction::E: return glm::vec2(0.5f, 0);
        case direction::W: return glm::vec2(-0.5f, 0);
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


std::vector<Entity> walls;

Entity guy;
static void initMaze();
void PrototypeGame::init(const uint8_t*, int) {
    LOGI("PrototypeGame initialisation begins...");

    sceneStateMachine.setup(gameThreadContext->assetAPI);

    faderHelper.init(camera);

    sceneStateMachine.start(Scene::Menu);

    walls.push_back(0);

    glm::vec2 is = glm::vec2((int)TRANSFORM(camera)->size.x,
        (int)TRANSFORM(camera)->size.y);
    glm::vec2 first = glm::vec2(-0.5 * MAZE_SIZE, -0.5 * MAZE_SIZE)
        + glm::vec2(0.5f, 0.5f);
    for (int i=0; i<MAZE_SIZE; i++) {
        for (int j=0; j<MAZE_SIZE; j++) {
            grid[i][j].e = theEntityManager.CreateEntityFromTemplate("cell");

            TRANSFORM(grid[i][j].e)->position = first + glm::vec2(i, j);
            grid[i][j].attr = cell_attibute::Out;

            for (int k=0; k<4; k++) {
                // always create north and east walls
                if ((k < 2) || (i==0 && k == 3) || (j==0 && k == 4)) {
                    Entity wall = theEntityManager.CreateEntityFromTemplate("wall");
                    ANCHOR(wall)->parent = grid[i][j].e;
                    ANCHOR(wall)->position = wallIdxToAnchor((direction::Enum)k);
                    ANCHOR(wall)->rotation = wallIdxToRotation((direction::Enum)k);
                    walls.push_back(wall);

                    grid[i][j].wallIndirect[k] = walls.size() - 1;
                } else if (k == 2) {
                    // south
                    grid[i][j].wallIndirect[k] = grid[i][j-1].wallIndirect[0];
                } else {
                    // west
                    grid[i][j].wallIndirect[k] = grid[i-1][j].wallIndirect[1];
                }
            }
        }
    }

    initMaze();

    guy = theEntityManager.CreateEntityFromTemplate("guy");
    TRANSFORM(guy)->position = first;

    theCollisionSystem.worldSize = TRANSFORM(camera)->size;

}
static float spawn = 1;
static bool mazeBuilt = false;

static bool buildMaze();



void PrototypeGame::tick(float dt) {
    spawn += dt;
    if (!mazeBuilt) {
        if (spawn >= 1) {
            spawn -= 1;
        }
        mazeBuilt = buildMaze();
    }

    if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_LEFT)) {
        ZSQD(guy)->directions.push_back(glm::vec2(-1.0f, 0.0));
    } else if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_RIGHT)) {
        ZSQD(guy)->directions.push_back(glm::vec2(1.0f, 0.0));
    }
    if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_UP)) {
        ZSQD(guy)->directions.push_back(glm::vec2(0.0f, 1.0));
    } else if (gameThreadContext->keyboardInputHandlerAPI->isKeyPressed(SDLK_DOWN)) {
        ZSQD(guy)->directions.push_back(glm::vec2(0.0f, -1.0));
    }

    for (int i=0; i<MAZE_SIZE; i++) {
        for (int j=0; j<MAZE_SIZE; j++) {
            RENDERING(grid[i][j].e)->color = attributeToColor(grid[i][j].attr);
        }
    }

    sceneStateMachine.update(dt);
}

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

            grid[d.x][d.y].attr = cell_attibute::Frontier;
        }
    }
}

void initMaze() {
    glm::ivec2 first(Random::Int(0, MAZE_SIZE - 1), Random::Int(0, MAZE_SIZE - 1));
    grid[first.x][first.y].attr = cell_attibute::In;
    in.push_back(first);
    updateFrontier(first);
}

#include "util/Draw.h"
bool buildMaze() {
    if (frontier.empty()) {
        return true;
    }

    // pick a new cell
    int idx = Random::Int(0, frontier.size() - 1);
    const Coords c = frontier[idx];

    std::vector<Coords> n = neighbours(c);
    n.erase(std::remove_if(n.begin(), n.end(), [] (const Coords& nb) {
        // remove if not in maze
        return std::find(in.begin(), in.end(), nb) == in.end();
    }), n.end());

    LOGF_IF(n.empty(), "Every frontier cell should have at least 1 neighbour in maze");
    Coords from = n[Random::Int(0, n.size() - 1)];

    in.push_back(c);
    grid[c.x][c.y].attr = cell_attibute::In;
    frontier.erase(frontier.begin() + idx);

    LOGI(__(c) << "->" << __(from));
    LOGF_IF(std::find(in.begin(), in.end(), from) == in.end(), "Uh");

    // update walls
    if (from.x < c.x) {
        int idx = grid[c.x][c.y].wallIndirect[direction::W];
        theEntityManager.DeleteEntity(walls[idx]);
        grid[c.x][c.y].wallIndirect[direction::W] = 0;
    }
    if (from.x > c.x) {
        int idx = grid[c.x][c.y].wallIndirect[direction::E];
        theEntityManager.DeleteEntity(walls[idx]);
        grid[c.x][c.y].wallIndirect[direction::E] = 0;
    }
    if (from.y < c.y) {
        int idx = grid[c.x][c.y].wallIndirect[direction::S];
        theEntityManager.DeleteEntity(walls[idx]);
        grid[c.x][c.y].wallIndirect[direction::S] = 0;
    }
    if (from.y > c.y)  {
        int idx = grid[c.x][c.y].wallIndirect[direction::N];
        theEntityManager.DeleteEntity(walls[idx]);
        grid[c.x][c.y].wallIndirect[direction::N] = 0;
    }

    Draw::Vec2(
        TRANSFORM(grid[from.x][from.y].e)->position,
        TRANSFORM(grid[c.x][c.y].e)->position - TRANSFORM(grid[from.x][from.y].e)->position,
        Color(0, 0, 1));

    updateFrontier(c);

    return false;
}
