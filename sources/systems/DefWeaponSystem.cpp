#include "DefWeaponSystem.h"

#include "base/Log.h"
#include "systems/TransformationSystem.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(DefWeaponSystem);

DefWeaponSystem::DefWeaponSystem() : ComponentSystemImpl<DefWeaponComponent>("DefWeapon") {
    DefWeaponComponent tc;
    componentSerializer.add(new Property<glm::vec2>("ellipse_param", OFFSET(ellipseParam, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("ellipse_angle_range", OFFSET(ellipseAngleRange, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("attack_mode_offset", OFFSET(attackModeOffset, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("max_angular_speed", OFFSET(maxAngularSpeed, tc), 0.001));
}

void DefWeaponSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(DefWeapon, a, swc)
        if (!swc->active)
            continue;
    	TransformationComponent* tc = TRANSFORM(a);
    	// compute angle between 'target' and parent worldPos
    	const TransformationComponent* ptc = TRANSFORM(tc->parent);
    	glm::vec2 diff(glm::rotate(swc->target, ptc->worldRotation) - ptc->worldPosition);
    	float angle = glm::atan2(diff.y, diff.x);
    	// angle is inside [-pi, pi], we want [-pi/2, 3pi/2]
    	if (angle < -glm::half_pi<float>())
    		angle += 2 * glm::pi<float>();
    	angle = glm::clamp(angle, swc->ellipseAngleRange.x, swc->ellipseAngleRange.y);

        // LOGI(angle-tc->rotation << ", " << glm::floor((angle - tc->rotation) / (2 * glm::pi<float>())))
        float positiveDiff = glm::mod(angle - swc->angle /*tc->rotation*/, 2 * glm::pi<float>());
        // consider both rotations
        float negativeDiff = - 2 * glm::pi<float>() + positiveDiff;

        if (glm::abs(negativeDiff) < glm::abs(positiveDiff))
            positiveDiff = negativeDiff;

    	swc->angle = glm::clamp(
            swc->angle +
            glm::sign(positiveDiff) * glm::min(swc->maxAngularSpeed * dt, glm::abs(positiveDiff)),
            swc->ellipseAngleRange.x, swc->ellipseAngleRange.y);

    	tc->position.x = swc->ellipseParam.x * glm::cos(swc->angle);
    	tc->position.y = swc->ellipseParam.y * glm::sin(swc->angle);

        if (swc->attack) {
            tc->rotation = glm::half_pi<float>() + swc->angle;
            tc->position += glm::rotate(swc->attackModeOffset, tc->rotation);
        } else {
            tc->rotation = swc->angle;
        }
    }
}

#if SAC_INGAME_EDITORS
void DefWeaponSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    DefWeaponComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "ellipse a", TW_TYPE_FLOAT, &tc->ellipseParam.x, "group=DefWeapon");
    TwAddVarRW(bar, "ellipse b", TW_TYPE_FLOAT, &tc->ellipseParam.y, "group=DefWeapon");
    TwAddVarRW(bar, "ellipse min angle", TW_TYPE_FLOAT, &tc->ellipseAngleRange.x, "group=DefWeapon");
    TwAddVarRW(bar, "ellipse max angle", TW_TYPE_FLOAT, &tc->ellipseAngleRange.y, "group=DefWeapon");
    TwAddVarRW(bar, "max angular speed", TW_TYPE_FLOAT, &tc->maxAngularSpeed, "group=DefWeapon");
    TwAddVarRW(bar, "DefWeapon target_x", TW_TYPE_FLOAT, &tc->target.x, "group=DefWeapon");
    TwAddVarRW(bar, "DefWeapon target_y", TW_TYPE_FLOAT, &tc->target.y, "group=DefWeapon");
}
#endif
