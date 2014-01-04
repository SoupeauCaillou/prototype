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

#define SAC_SELECTION_SYSTEM 1

#include "SelectionSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "api/KeyboardInputHandlerAPI.h"
#include "base/TouchInputManager.h"

INSTANCE_IMPL(SelectionSystem);

SelectionSystem::SelectionSystem() : ComponentSystemImpl<SelectionComponent>("Selection"), kbApi(0) {
    SelectionComponent tc;
    componentSerializer.add(new Property<bool>("enabled", OFFSET(enabled, tc)));
    componentSerializer.add(new EntityProperty("icon", OFFSET(icon, tc)));
    componentSerializer.add(new Property<int>("key_code", OFFSET(keyScanCode, tc)));
    componentSerializer.add(new Property<bool>("selected", OFFSET(selected, tc)));
}

void SelectionSystem::DoUpdate(float dt) {
    Entity newSelection = 0;

    for (auto p: components) {
        auto entity = p.first;
        auto* sc = p.second;
        if (!sc->enabled) {
            if (sc->icon) {
                RENDERING(sc->icon)->show = false;
            }
            sc->selected = sc->newlySelected = false;
        } else {
            // exclusive selection
            if (BUTTON(entity)->clicked
                || (kbApi && sc->keyScanCode >= 0 && kbApi->isKeyReleased(sc->keyScanCode))
                || (theJoystickManager.hasClicked(0, sc->joystickBtn))) {
                if (!sc->selected) {
                    newSelection = entity;
                    if (sc->icon) {
                        ANCHOR(sc->icon)->parent = newSelection;
                        RENDERING(sc->icon)->show = true;
                    }
                    sc->selected = sc->newlySelected = true;
                } else {
                    if (sc->icon) {
                        RENDERING(sc->icon)->show = false;
                    }
                    sc->selected = sc->newlySelected = false;   
                }
                break;
            } else {
                sc->newlySelected = false;
            }
        }
    }

    if (newSelection) {
        for (auto p: components) {
            auto entity = p.first;

            if (entity != newSelection) {
                auto* sc = p.second;
                if (sc->icon) {
                    RENDERING(sc->icon)->show = false;
                }
                sc->selected = sc->newlySelected = false;
            }
        }
    }
}

