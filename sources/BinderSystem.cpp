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

#include "BinderSystem.h"

#include "util/Tuning.h"

INSTANCE_IMPL(BinderSystem);

BinderSystem::BinderSystem() : ComponentSystemImpl<BinderComponent>(HASH("Binder", 0x7195d9b4)) {}

void BinderSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Binder, e, comp)
    const float* varFrom = static_cast<float*>((float*)comp->from.system->componentAsVoidPtr(e) + comp->from.offset);
    float* varTo = static_cast<float*>((float*)comp->to.system->componentAsVoidPtr(e) + comp->to.offset);

    float t = comp->from.interval.invLerp(*varFrom);
    *varTo = comp->to.interval.lerp(t);
}
}
