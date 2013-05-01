#include "DefWeaponSystem.h"

#include "base/Log.h"
#include "base/EntityManager.h"
#include "systems/TransformationSystem.h"
#include "systems/ADSRSystem.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(DefWeaponSystem);

DefWeaponSystem::DefWeaponSystem() : ComponentSystemImpl<DefWeaponComponent>("DefWeapon") {
    DefWeaponComponent tc;
    componentSerializer.add(new Property<glm::vec2>("ellipse_param", OFFSET(ellipseParam, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("ellipse_angle_range", OFFSET(ellipseAngleRange, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("attack_mode_offset", OFFSET(attackModeOffset, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("max_angular_speed", OFFSET(maxAngularSpeed, tc), 0.001));
    componentSerializer.add(new Property<float>("mode_transition_duration", OFFSET(modeTransitionDuration, tc), 0.001));
}

static Entity createTransitionEntity(DefWeaponComponent* dfc);

void DefWeaponSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(DefWeapon, a, swc)
        ADSRComponent* trans = 0;
        auto tranIt = transition.find(a);
        if (tranIt == transition.end()) {
            Entity transitionEntity = createTransitionEntity(swc);
            transition.insert(std::make_pair(a, transitionEntity));
            trans = ADSR(transitionEntity);
        } else {
            trans = ADSR(tranIt->second);
        }

        if (!swc->active) {
            trans->active = false;
            continue;
        } else {
            trans->active = swc->attack;
        }

    	TransformationComponent* tc = TRANSFORM(a);
    	// compute angle between 'target' and parent worldPos
    	const TransformationComponent* ptc = TRANSFORM(tc->parent);
    	glm::vec2 diff(glm::rotate(swc->target, ptc->worldRotation) - ptc->worldPosition);
    	float angle = glm::atan2(diff.y, diff.x);
    	// angle is inside [-pi, pi], we want [-pi/2, 3pi/2]
    	if (angle < -glm::half_pi<float>())
    		angle += 2 * glm::pi<float>();
    	angle = glm::clamp(angle, swc->ellipseAngleRange.x, swc->ellipseAngleRange.y);

        float positiveDiff = glm::mod(angle - swc->angle, 2 * glm::pi<float>());
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

        tc->rotation = trans->value * glm::half_pi<float>() + swc->angle;
        tc->position += glm::rotate(trans->value * swc->attackModeOffset, tc->rotation);
    }

    // TODO remove useless transitionEntity
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

static Entity createTransitionEntity(DefWeaponComponent* dfc) {
    Entity e = theEntityManager.CreateEntity("def_transition");
    ADD_COMPONENT(e, ADSR);
    ADSR(e)->idleValue = 0;
    ADSR(e)->attackValue = ADSR(e)->sustainValue = 1;
    ADSR(e)->attackTiming = ADSR(e)->releaseTiming = dfc->modeTransitionDuration;
    ADSR(e)->decayTiming = 0;
    return e;
}
