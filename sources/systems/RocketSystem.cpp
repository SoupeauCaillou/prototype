/*
    This file is part of Prototype.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer

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
#include "RocketSystem.h"

#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"

#include "util/drawVector.h"

INSTANCE_IMPL(RocketSystem);

RocketSystem::RocketSystem() : ComponentSystemImpl<RocketComponent>("Rocket") {
    RocketComponent pc;
    componentSerializer.add(new Property<float>("rocket_weight", OFFSET(rocketWeight, pc), 0.001f));
    componentSerializer.add(new Property<float>("pushing_force", OFFSET(pushingForce, pc), 0.001f));
    componentSerializer.add(new Property<float>("engine_consumption", OFFSET(engineConsumption, pc), 0.001f));
    componentSerializer.add(new Property<float>("tank_occupied", OFFSET(tankOccupied, pc), 0.001f));
    componentSerializer.add(new Property<float>("tank_max", OFFSET(tankMax, pc), 0.001f));
    componentSerializer.add(new Property<float>("gasoline_weight", OFFSET(gasolineWeight, pc), 0.001f));
}

void RocketSystem::DoUpdate(float dt) {
    static Entity v = 0;
    FOR_EACH_ENTITY_COMPONENT(Rocket, e, rc)
        if (rc->tankOccupied > 0) {
            rc->tankOccupied -= rc->engineConsumption * dt;
            if (rc->tankOccupied < 0)
                rc->tankOccupied = 0;
            if (v)
                theEntityManager.DeleteEntity(v);
            v = drawVector(TRANSFORM(e)->position, glm::vec2(0.f, rc->pushingForce));
            PhysicsComponent *phc = PHYSICS(e);
            phc->addForce(glm::vec2(0.f, rc->pushingForce), glm::vec2(0.f,0.f), dt);
            phc->mass = rc->gasolineWeight * rc->tankOccupied + rc->rocketWeight;
        }
    }
}

#ifdef SAC_INGAME_EDITORS
void RocketSystem::addEntityPropertiesToBar(Entity entity, CTwBar* bar) {
    RocketComponent* rc = Get(entity, false);
    if (!rc) return;
    TwAddVarRW(bar, "rocket_weight", TW_TYPE_FLOAT, &rc->rocketWeight, "group=Rocket precision=2 step=0,01");
    TwAddVarRW(bar, "pushing_force", TW_TYPE_FLOAT, &rc->pushingForce, "group=Rocket precision=2 step=0,01");
    TwAddVarRW(bar, "engine_consumption", TW_TYPE_FLOAT, &rc->engineConsumption, "group=Rocket precision=2 step=0,01");
    TwAddVarRW(bar, "tank_occupied", TW_TYPE_FLOAT, &rc->tankOccupied, "group=Rocket precision=2 step=0,01");
    TwAddVarRW(bar, "tank_max", TW_TYPE_FLOAT, &rc->tankMax, "group=Rocket precision=2 step=0,01");
    TwAddVarRW(bar, "gasoline_weight", TW_TYPE_FLOAT, &rc->gasolineWeight, "group=Rocket precision=2 step=0,01");
}
#endif