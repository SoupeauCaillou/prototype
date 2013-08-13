/*
    This file is part of Dogtag.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer

    Dogtag is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Dogtag is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dogtag.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "MorpionGridSystem.h"
#include "PrototypeGame.h"

#include "systems/TransformationSystem.h"
#include "systems/TicTacToeSystem.h"

#include "base/PlacementHelper.h"

INSTANCE_IMPL(MorpionGridSystem);

MorpionGridSystem::MorpionGridSystem() : ComponentSystemImpl<MorpionGridComponent>("MorpionGrid") {
    MorpionGridComponent tc;
    componentSerializer.add(new Property<int>("i", OFFSET(i, tc)));
    componentSerializer.add(new Property<int>("j", OFFSET(j, tc)));
}

std::vector<Entity> MorpionGridSystem::getCellsForMiniMorpion(int inI, int inJ, MorpionGridComponent::E_Type type) {
    std::vector<Entity> v;

    auto * ttt = theTicTacToeSystem.getAllComponents().begin()->second;

    int startI = (inI / 3) * 3;
    int startJ = (inJ / 3) * 3;
    for (int i = startI; i < startI + 3; ++i) {
        for (int j = startJ; j < startJ + 3; ++j) {
            if (MORPION_GRID(ttt->grid[i * 9 + j])->type & type) {
                v.push_back(ttt->grid[i * 9 + j]);
            }
        }
    }
    return v;
}

Entity MorpionGridSystem::getEntityAtPosition(int i, int j) {
    Entity result = 0;
    theMorpionGridSystem.forEachECDo([&] (Entity e, MorpionGridComponent *mc) -> void {
        if (mc->i == i && mc->j == j)
            result = e;
    });
    return result;
}

std::vector<Entity> MorpionGridSystem::nextPlayableCells(Entity currentCell) {
    std::vector<Entity> v;

    if (currentCell) {
        int startI = (MORPION_GRID(currentCell)->i % 3) * 3;
        int startJ = (MORPION_GRID(currentCell)->j % 3) * 3;
        v = getCellsForMiniMorpion(startI, startJ, MorpionGridComponent::Available);
    }

    //if currentCell is null (first turn) OR if all the next cells are already played, next player can play everywhere!
    if (v.empty()) {
        auto * ttt = theTicTacToeSystem.getAllComponents().begin()->second;
        for (int i = 0; i < 81; ++i) {
            if (MORPION_GRID(ttt->grid[i])->type == MorpionGridComponent::Available) {
                v.push_back(ttt->grid[i]);
            }
        }
    }
    return v;
}

glm::vec2 MorpionGridSystem::gridCellToPosition(int i, int j) {
    auto s = PlacementHelper::ScreenSize;

    auto topLeftOriginMinusOffset = .45f * glm::vec2(-s.x, s.y);
    return topLeftOriginMinusOffset + glm::vec2(i / 9. * s.x, - j / 9. * s.y);
}

bool MorpionGridSystem::isMiniMorpionFinished(int i, int j) {
    static int lookingFor[] =  {0,0,1,0,2,0,
                                0,0,0,1,0,2,
                                0,0,1,1,2,2,
                                1,0,1,1,1,2,
                                2,0,1,1,0,2,
                                2,0,2,1,2,2,
                                0,1,1,1,2,1,
                                0,2,1,2,2,2};
    int startI = (i / 3) * 3;
    int startJ = (j / 3) * 3;
    //Looking for 2 players
    for (int player=0; player<2; ++player) {
        std::vector<Entity> v = getCellsForMiniMorpion(i, j, player ? MorpionGridComponent::Player1:MorpionGridComponent::Player2);
        // 8 possibilities
        for (int t=0; t<8; ++t) {
            int found=0;
            // We check cell by cell
            for (int u=0; u<6; u+=2) {
                for (auto e : v){
                    if (MORPION_GRID(e)->i == startI+lookingFor[t*6 + u] && MORPION_GRID(e)->j == startJ+lookingFor[t*6 + u + 1]) {
                        ++found;
                        break;
                    }
                }
                // if we don't have a 100% rate we stop the try
                if (found != (u/2)+1)
                    break;
            }
            // if we found a full line
            if (found == 3)
                return true;
        }
    }
    return false;
}

bool MorpionGridSystem::isMaxiMorpionFinished() {
    for (auto it: components) {
        auto* mgc = it.second;
        if (mgc->type == MorpionGridComponent::Available) {
            return false;
        }
    }
    return true;
}

void MorpionGridSystem::DoUpdate(float ) {
    for (auto it: components) {
        const Entity e = it.first;
        auto* mgc = it.second;

        switch (mgc->type) {
            case MorpionGridComponent::Playable:
                RENDERING(e)->color = Color(0., 0., 1., .5);
                break;
            case MorpionGridComponent::Available:
                RENDERING(e)->color = Color(0., 0., 0., 1.);
                break;
            case MorpionGridComponent::Player1:
                RENDERING(e)->color = Color(1., 0., 0., 1.);
                break;
            case MorpionGridComponent::Player2:
                RENDERING(e)->color = Color(0., 1., 0., 1.);
                break;
            case MorpionGridComponent::Lost:
                RENDERING(e)->color = Color(0.9, 0.5, 0.5, 1.);
                break;
            default:
                break;

        }
    }
}
