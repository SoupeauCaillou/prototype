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
#include "HealthSystem.h"
#include "util/SerializerProperty.h"

INSTANCE_IMPL(HealthSystem);

HealthSystem::HealthSystem() : ComponentSystemImpl<HealthComponent>(HASH("Health", 0x43077850)) {
    HealthComponent tc;
    componentSerializer.add(new Property<int>(HASH("current_hp", 0x8d313010), OFFSET(currentHP, tc)));
    componentSerializer.add(new Property<int>(HASH("max_hp", 0x1312bf8c), OFFSET(maxHP, tc)));
}

void HealthSystem::DoUpdate(float) {
}
