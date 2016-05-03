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
#include "EquipmentSystem.h"
#include "PlayerSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "util/SerializerProperty.h"

#include <sac/tweak.h>

INSTANCE_IMPL(EquipmentSystem);

EquipmentSystem::EquipmentSystem() : ComponentSystemImpl<EquipmentComponent>(HASH("Equipment", 0xbbc78eb2)) {
    EquipmentComponent tc;
    componentSerializer.add(new EntityProperty(HASH("left_hand", 0x575b422f), OFFSET(hand.left, tc)));
    componentSerializer.add(new EntityProperty(HASH("right_hand", 0xa08683b3), OFFSET(hand.right, tc)));
}

void EquipmentSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Equipment, entity, pc)
        for (int i=0; i<2; i++) {
            if (pc->hands[i]) {
                ANCHOR(pc->hands[i])->parent = entity;
            }
        }
    END_FOR_EACH()
}
