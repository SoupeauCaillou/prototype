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



#include "BulletSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"
#include "SoldierSystem.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl<BulletComponent>("Bullet") {
    BulletComponent tc;
    componentSerializer.add(new Property<float>("damage", OFFSET(damage, tc), 0.001));
}

void BulletSystem::DoUpdate(float dt) {
    std::vector<Entity> toDelete;
    for (auto p: components) {
        auto entity = p.first;
        auto* bc = p.second;
        const auto* cc = COLLISION(entity);

        if (cc->rayTestDone) {
            // Display
            Entity bullet = bc->display;
            const glm::vec2 weaponNose(TRANSFORM(bullet)->position);
            RENDERING(bullet)->show = true;
            TRANSFORM(bullet)->position = (weaponNose + cc->collisionAt)*0.5f;
            TRANSFORM(bullet)->size.x = glm::length(cc->collisionAt - weaponNose);
            TRANSFORM(bullet)->rotation = TRANSFORM(entity)->rotation;

            auto* sc = theSoldierSystem.Get(cc->collidedWithLastFrame, false);
            if (sc) {
                sc->health -= p.second->damage;
            } else {

            }
            toDelete.push_back(entity);
        }
    }

    for (auto e: toDelete)
        theEntityManager.DeleteEntity(e);
}

