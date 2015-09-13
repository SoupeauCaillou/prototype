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



#include "BackgroundSystem.h"

#include "base/EntityManager.h"
#include "systems/ParticuleSystem.h"
#include "systems/TransformationSystem.h"
#include "util/SerializerProperty.h"

#include "PrototypeGame.h"

INSTANCE_IMPL(BackgroundSystem);

BackgroundSystem::BackgroundSystem() : ComponentSystemImpl<BackgroundComponent>(HASH("Background", 0x4d33e992)) {
    BackgroundComponent bg;
    componentSerializer.add(new Property<float>(HASH("burning_speed", 0x18f19e94), OFFSET(burningSpeed, bg), 0.001f));
}

void BackgroundSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Background, e, bg)
        float loss = PARTICULE(e)->emissionRate / 50.f * dt / bg->burningSpeed;
        TRANSFORM(e)->size.y -= loss;
        TRANSFORM(e)->position.y -= loss / 2.f;
        // burned tree
        if (TRANSFORM(e)->size.y <= 0.f) {
            theEntityManager.DeleteEntity(e);
        }
    }
}
