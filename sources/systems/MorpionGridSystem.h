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
    along with Dogtag. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "systems/System.h"

class PrototypeGame;

struct MorpionGridComponent {
    MorpionGridComponent() : i(0), j(0), type(Available) {}

    int i;
    int j;

    enum E_Type {
        Available = 1 << 1,
        Playable = 1 << 2,
        Player1 = 1 << 3,
        Player2 = 1 << 4,
        Lost = 1 << 5,
        All = Available | Playable | Player1 | Player2 | Lost,
    };

    E_Type type;
};

#define theMorpionGridSystem MorpionGridSystem::GetInstance()
#define MORPION_GRID(e) theMorpionGridSystem.Get(e)

UPDATABLE_SYSTEM(MorpionGrid)
    public:
        Entity positionToGridCell(const glm::vec2 & position);
        std::vector<Entity> getCellsForMiniMorpion(int inI, int inJ, MorpionGridComponent::E_Type type);
        std::vector<Entity> nextPlayableCells(Entity currentCell);
        glm::vec2 gridCellToPosition(int i, int j);
        Entity getEntityAtPosition(int i, int j);

        bool isMiniMorpionFinished(int i, int j);
        bool isMaxiMorpionFinished();

        PrototypeGame* game;

};
