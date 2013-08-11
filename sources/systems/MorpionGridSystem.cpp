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

#include "base/PlacementHelper.h"

INSTANCE_IMPL(MorpionGridSystem);

MorpionGridSystem::MorpionGridSystem() : ComponentSystemImpl<MorpionGridComponent>("MorpionGrid") {
    MorpionGridComponent tc;
    componentSerializer.add(new Property<int>("i", OFFSET(i, tc)));
    componentSerializer.add(new Property<int>("j", OFFSET(j, tc)));
}

std::vector<Entity> MorpionGridSystem::nextPlayableCells(Entity currentCell) {
    std::vector<Entity> v;

    if (currentCell) {
        int startI = (MORPION_GRID(currentCell)->i % 3) * 3;
        int startJ = (MORPION_GRID(currentCell)->j % 3) * 3;
        for (int i = startI; i < startI + 3; ++i) {
            for (int j = startJ; j < startJ + 3; ++j) {
                if (RENDERING(game->grid[i * 9 + j])->color == Color(0., 0., 0.)) {
                    v.push_back(game->grid[i * 9 + j]);
                }
            }
        }
    }

    //if currentCell is null (first turn) OR if all the next cells are already played, next player can play everywhere!
    if (v.empty()) {
        for (int i = 0; i < 81; ++i) {
            if (RENDERING(game->grid[i])->color == Color(0., 0., 0.)) {
                v.push_back(game->grid[i]);
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


void MorpionGridSystem::DoUpdate(float ) {
}
