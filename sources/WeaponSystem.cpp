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



#include "WeaponSystem.h"
#include "BulletSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"
#include "util/Random.h"
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(WeaponSystem);

WeaponSystem::WeaponSystem() : ComponentSystemImpl<WeaponComponent>("Weapon") {
    WeaponComponent tc;
    componentSerializer.add(new Property<float>("fire_speed", OFFSET(fireSpeed.value, tc), 0.001));
    componentSerializer.add(new Property<float>("reload_speed", OFFSET(reloadSpeed.value, tc), 0.001));
    componentSerializer.add(new Property<int>("bullet_per_shot", OFFSET(bulletPerShot, tc)));
    componentSerializer.add(new Property<float>("bullet_damage", OFFSET(bulletDamage, tc), 0.001));
    componentSerializer.add(new Property<float>("precision", OFFSET(precision, tc), 0.001));
    componentSerializer.add(new StringProperty("bullet_template", OFFSET(bulletTemplate, tc)));
    componentSerializer.add(new Property<bool>("fire", OFFSET(fire, tc)));
    componentSerializer.add(new Property<bool>("reload", OFFSET(reload, tc)));
    componentSerializer.add(new Property<int>("ammo_left_in_clip", OFFSET(ammoLeftInClip, tc)));
}

void WeaponSystem::DoUpdate(float dt) {
    for (auto p: components) {
        auto entity = p.first;
        auto* wc = p.second;

        wc->fireSpeed.accum = glm::min(wc->fireSpeed.accum + wc->fireSpeed.value * dt, 1.0f);

        if (wc->fire) {
            wc->reload = false;
            wc->reloadSpeed.accum = 0;
            if (!wc->ammoLeftInClip && false) {
                LOGI_EVERY_N(60, "No more ammo");
            } else {
                // nose
                const auto* tc = TRANSFORM(entity);
                glm::vec2 nose = tc->position + glm::rotate(tc->size * glm::vec2(0.5f, 0.0f), tc->rotation);

                while (wc->fireSpeed.accum >= 1 && wc->ammoLeftInClip) {
                    wc->fireSpeed.accum -= 1;
                    wc->ammoLeftInClip -= 1;

                    for (int i=0; i<wc->bulletPerShot; i++) {
                        // Collision
                        Entity bColl = theEntityManager.CreateEntityFromTemplate("bullet");
                        TRANSFORM(bColl)->position = nose;
                        TRANSFORM(bColl)->rotation = tc->rotation + Random::Float(-0.5f * wc->precision, 0.5f * wc->precision);
                        BULLET(bColl)->damage = wc->bulletDamage;

                        Entity b = BULLET(bColl)->display = theEntityManager.CreateEntityFromTemplate(wc->bulletTemplate);
                        TRANSFORM(b)->position = nose;
                    }
                }
            }
        } else if (wc->reload) {
            wc->fire = false;
            wc->fireSpeed.accum = 0;
            // random ammo for now
            wc->reloadSpeed.accum += wc->reloadSpeed.value * dt;
            while (wc->reloadSpeed.accum > 1) {
                wc->ammoLeftInClip += 1;
                wc->reloadSpeed.accum -= 1;
            }
        } else {
            wc->fire = false;
            wc->reload = false;
        }
    }
}

