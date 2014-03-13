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


#if SAC_INGAME_EDITORS && SAC_DEBUG

#include "PrototypeDebugConsole.h"

#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"

#include "base/Log.h"
#include "base/EntityManager.h"

#include "PrototypeGame.h"

PrototypeGame* PrototypeDebugConsole::game = 0;

void PrototypeDebugConsole::init(PrototypeGame* g) {
    game = g;

    DebugConsole::RegisterMethod("Move all sheep to final zone", moveAllSheepToFinalZone);
}

void PrototypeDebugConsole::moveAllSheepToFinalZone(void*) {
    Entity zone = game->levelLoader.arrivalZone;
    for (auto s : game->levelLoader.sheep) {
        TRANSFORM(s)->position = TRANSFORM(zone)->position;
    }
}

#endif
