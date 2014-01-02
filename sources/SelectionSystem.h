/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "systems/System.h"

#if !SAC_SELECTION_SYSTEM
#define SELECTION_CONST const
#endif

class KeyboardInputHandlerAPI;

struct SelectionComponent {
    SelectionComponent() : enabled(false), icon(0), keyScanCode(-1), selected(false), newlySelected(false) {}

    bool enabled;
    Entity icon;
    int keyScanCode;

    /*SELECTION_CONST*/ bool selected, newlySelected;
};

#define theSelectionSystem SelectionSystem::GetInstance()
#if SAC_DEBUG
#define SELECTION(e) theSelectionSystem.Get(e,true,__FILE__,__LINE__)
#else
#define SELECTION(e) theSelectionSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Selection)
public:
    KeyboardInputHandlerAPI* kbApi;
};
