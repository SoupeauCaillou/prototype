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



#include "ProjectileSystem.h"

#include "SoldierSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"

INSTANCE_IMPL(ProjectileSystem);

ProjectileSystem::ProjectileSystem() : ComponentSystemImpl<ProjectileComponent>("Projectile") {
    ProjectileComponent tc;
    componentSerializer.add(new Property<float>("damage", OFFSET(damage, tc)));
}

void ProjectileSystem::DoUpdate(float) {
    std::list<Entity> toRemove;

    for (auto p: components) {
        Entity e = p.first;
        auto* pc = p.second;

        // check for collision
        auto* cc = COLLISION(e);
        if (cc->collidedWithLastFrame) {
            auto* sc = theSoldierSystem.Get(cc->collidedWithLastFrame);
            if (sc) {
                LOGI("Projectile " << theEntityManager.entityName(e) << " hit " << theEntityManager.entityName(cc->collidedWithLastFrame));
                sc->health -= pc->damage;
            }

            ADD_COMPONENT(e, Anchor);
            ANCHOR(e)->parent = cc->collidedWithLastFrame;
            ANCHOR(e)->position = TRANSFORM(e)->position - TRANSFORM(cc->collidedWithLastFrame)->position;
            ANCHOR(e)->rotation = TRANSFORM(e)->rotation - TRANSFORM(cc->collidedWithLastFrame)->rotation;

            toRemove.push_back(e);
        }
    }

    for (auto e: toRemove) {
        theEntityManager.RemoveComponent(e, PhysicsSystem::GetInstancePointer());
        theEntityManager.RemoveComponent(e, CollisionSystem::GetInstancePointer());
        theEntityManager.RemoveComponent(e, ProjectileSystem::GetInstancePointer());
    }
}

