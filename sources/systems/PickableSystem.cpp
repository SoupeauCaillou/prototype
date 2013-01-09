/*
    This file is part of Brikwars.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    RecursiveRunner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    RecursiveRunner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "PickableSystem.h"
#include "systems/TransformationSystem.h"
#include "util/IntersectionUtil.h"
#include "base/TouchInputManager.h"

INSTANCE_IMPL(PickableSystem);

PickableSystem::PickableSystem() : ComponentSystemImpl<PickableComponent>("Pickable") {

}

void PickableSystem::DoUpdate(float) {
    bool click = !theTouchInputManager.isTouched(0) &&
                theTouchInputManager.wasTouched(0);
    FOR_EACH_ENTITY_COMPONENT(Pickable, entity, pick)
        if (!pick->enable) {
            pick->picked = false;
        } else if (click) {
            const Vector2& v = theTouchInputManager.getTouchLastPosition(0);
            pick->picked = (IntersectionUtil::rectangleRectangle(v, Vector2::Zero, 0,
                TRANSFORM(entity)->worldPosition, TRANSFORM(entity)->size, TRANSFORM(entity)->worldRotation));
            if (pick->picked) {
                std::cout << entity << " picked!" << std::endl;
            }
        }
    }
}

