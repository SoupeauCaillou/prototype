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

#include "FlagSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "SoldierSystem.h"
#include "TeamSystem.h"

#include "util/IntersectionUtil.h"

INSTANCE_IMPL(FlagSystem);

FlagSystem::FlagSystem() : ComponentSystemImpl<FlagComponent>("Flag") {

}

void FlagSystem::DoUpdate(float) {
    auto& soldiers = theSoldierSystem.getAllComponents();
    for (auto& p: components) {
        Entity e = p.first;

        auto owner = ANCHOR(e)->parent;
        if (owner) {
            if (SOLDIER(owner)->health <= 0) {
                ANCHOR(e)->parent = 0;
            } else {
                Entity spawn = TEAM(SOLDIER(owner)->team)->spawn;
                if (IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(spawn))) {
                    TEAM(SOLDIER(owner)->team)->flagCaptured = true;
                }
            }
        } else {
            for (auto it: soldiers) {
                const auto* tc = TRANSFORM(it.first);
                if (IntersectionUtil::rectangleRectangle(TRANSFORM(e), tc)) {
                    ANCHOR(e)->parent = it.first;
                    break;
                }
            }
        }
    }
}

