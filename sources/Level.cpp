#include "Level.h"
#include "util/HexSpatialGrid.h"
#include "util/FileBufferHelper.h"
#include "base/Log.h"
#include "base/EntityManager.h"
#include "SacHelloWorldGame.h"
#include "systems/GridSystem.h"

/*
File level description:
N,M
XXXXXXXXXXXXX
XXXXXXXXXXXXX
XXXXXXXXXXXXX
[...]

N = grid width
M = grid height
XXXX = grid content
    - .: empty
    - X: block
    - S: start point
    - E: end point

Number of ships = number of start point
Number of E must be >= S
*/

HexSpatialGrid* Level::load(const FileBuffer& fb, bool createEntities) {
    FileBufferHelper h;
    HexSpatialGrid* grid = 0;
    int n,m;
    int row = 0;

    SpatialGrid::Iterate::Result result;

    while (true) {
        const char* line = h.line(fb);
        if (!line)
            break;
        if (line[0] == '#' || line[0] == '\0')
            continue;
        if (grid == 0) {
            // parse N,M
            int count = sscanf(line, "%d,%d", &n, &m);
            if (count != 2) {
                LOGE("Invalid line with grid size '" << line << '(' << count << " integers parsed");
                return NULL;
            }
            grid = new HexSpatialGrid(n, m, 2.6);
            result = grid->iterate(invalidGridPos);
        } else {
            // try to parse M int
            LOGE_IF((int)strlen(line) != n, "Invalid line '" << line << "' for row #" << row);
            for (int i=0; i<n; i++) {
                const char* entity[2] = {0, 0};
                bitfield_t type = 0;
                switch (line[i]) {
                    case '.': entity[0] = "field/cell_grass"; type = Case::Empty; break;
                    case 'X': entity[0] = "field/cell_rock"; type = Case::Rock; break;
                    case 'E': entity[0] = "field/cell_end"; type = Case::End; break;
                    case 'D':
                        entity[0] = "field/cell_grass";
                        entity[1] = "dog";
                        type = Case::Dog;
                        break;
                    case 'S':
                        entity[0] = "field/cell_grass";
                        entity[1] = "sheep";
                        type = Case::Start;
                        break;
                }
                {
                    Entity e = theEntityManager.CreateEntityFromTemplate(createEntities ? entity[0] : "field/cell_grass");
                    grid->addEntityAt(e, result.pos, true);
                    if (!createEntities) {
                        GRID(e)->type = type;
                    }
                }
                if (createEntities) {
                    Entity e = theEntityManager.CreateEntityFromTemplate(entity[1]);
                    grid->addEntityAt(e, result.pos, true);
                }
                result = grid->iterate(result.pos);
            }
            row++;
        }
    }

    return grid;
}
