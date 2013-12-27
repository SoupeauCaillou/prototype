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



#include "MessageSystem.h"

INSTANCE_IMPL(MessageSystem);

MessageSystem::MessageSystem() : ComponentSystemImpl<MessageComponent>("Message") {
    MessageComponent tc;
    componentSerializer.add(new Property<int>("type", OFFSET(type, tc)));
    componentSerializer.add(new Property<int>("new_state", OFFSET(newState, tc)));
    componentSerializer.add(new EntityProperty("soldier", OFFSET(soldier, tc)));
    componentSerializer.add(new Property<float>("health", OFFSET(health, tc)));
}

void MessageSystem::DoUpdate(float) {
}

