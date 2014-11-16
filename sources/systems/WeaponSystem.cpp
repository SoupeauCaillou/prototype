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
#include "systems/RenderingSystem.h"
#include "util/Random.h"
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(WeaponSystem);

WeaponSystem::WeaponSystem() : ComponentSystemImpl<WeaponComponent>(HASH("Weapon", 0x6249dfd3)) {
    WeaponComponent tc;
    componentSerializer.add(new Property<float>(HASH("fire_speed", 0xa80f9772), OFFSET(fireSpeed.value, tc), 0.001));
    componentSerializer.add(new Property<float>(HASH("reload_speed", 0x84b069fa), OFFSET(reloadSpeed.value, tc), 0.001));
    componentSerializer.add(new Property<int>(HASH("bullet_per_shot", 0x985327cf), OFFSET(bulletPerShot, tc)));
    componentSerializer.add(new Property<float>(HASH("bullet_speed", 0x548ea5fa), OFFSET(bulletSpeed, tc), 0.001));
    componentSerializer.add(new Property<float>(HASH("precision", 0x9170e688), OFFSET(precision, tc), 0.001));
    componentSerializer.add(new Property<bool>(HASH("fire", 0x3b3b834f), OFFSET(fire, tc)));
    componentSerializer.add(new Property<bool>(HASH("reload", 0xe5209b7e), OFFSET(reload, tc)));
    componentSerializer.add(new Property<int>(HASH("ammo_left_in_clip", 0x6f971ad7), OFFSET(ammoLeftInClip, tc)));
    componentSerializer.add(new Property<float>(HASH("heat_up_per_bullet", 0x9a8421d6), OFFSET(heatupPerBullet, tc)));
    componentSerializer.add(new Property<float>(HASH("cooling_speed", 0x3559848), OFFSET(coolingSpeed, tc)));

}

void WeaponSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Weapon, entity, wc)
        RENDERING(entity)->color.r = wc->_mustCoolDown;
        if (wc->fire) {
            wc->reload = false;
            wc->reloadSpeed.accum = 0;

            if (!wc->ammoLeftInClip) {
                LOGI_EVERY_N(60, "No more ammo");
            } else if (!wc->_mustCoolDown) {
                wc->fireSpeed.accum += wc->fireSpeed.value * dt;

                // nose
                const auto* tc = TRANSFORM(entity);
                glm::vec2 nose = tc->position + glm::rotate(tc->size * glm::vec2(0.5f, 0.0f), tc->rotation);

                while (wc->fireSpeed.accum > 1 && wc->ammoLeftInClip) {
                    wc->fireSpeed.accum -= 1;
                    wc->ammoLeftInClip -= 1;
                    wc->_hot += wc->heatupPerBullet;

                    for (int i=0; i<wc->bulletPerShot; i++) {
                        // Collision
                        Entity bColl = theEntityManager.CreateEntityFromTemplate("bullet");
                        TRANSFORM(bColl)->position = nose;
                        TRANSFORM(bColl)->rotation = tc->rotation + Random::Float(-0.5f * wc->precision, 0.5f * wc->precision);
                    }
                }
                wc->_mustCoolDown = (wc->_hot >= 1.0f);
            } else {
              wc->_hot = glm::max(0.0f, wc->_hot - wc->coolingSpeed * dt);
            }
        } else if (wc->reload) {
            wc->fire = false;
            wc->fireSpeed.accum = 0;
            // random ammo for now
            wc->reloadSpeed.accum += wc->reloadSpeed.value * dt;
            while (wc->reloadSpeed.accum > 1) {
                wc->ammoLeftInClip += 1;
                wc->reloadSpeed.accum -= 1;
                wc->_hot = 0;
                wc->_mustCoolDown = false;
            }
        } else {
            wc->_hot = glm::max(0.0f, wc->_hot - wc->coolingSpeed * dt);
            if (wc->_mustCoolDown && wc->_hot <= 0.0f)
                wc->_mustCoolDown = false;
            wc->fire = false;
            wc->reload = false;
            wc->fireSpeed.accum = glm::min(1.0f, wc->fireSpeed.accum + wc->fireSpeed.value * dt);
        }
    }
}

