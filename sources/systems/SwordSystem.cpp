#include "SwordSystem.h"

#include "base/Log.h"
#include "systems/TransformationSystem.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(SwordSystem);

SwordSystem::SwordSystem() : ComponentSystemImpl<SwordComponent>("Sword") {
    SwordComponent tc;
    componentSerializer.add(new Property<glm::vec2>("ellipse_param", OFFSET(ellipseParam, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("ellipse_angle_range", OFFSET(ellipseAngleRange, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("max_angular_speed", OFFSET(maxAngularSpeed, tc), 0.001));
}

void SwordSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Sword, a, swc)
    	TransformationComponent* tc = TRANSFORM(a);
    	// compute angle between 'target' and parent worldPos
    	const TransformationComponent* ptc = TRANSFORM(tc->parent);
    	glm::vec2 diff(glm::rotate(swc->target, ptc->worldRotation) - ptc->worldPosition);
    	float angle = glm::atan2(diff.y, diff.x);
    	// angle is inside [-pi, pi], we wand [-pi/2, 3pi/2]
    	if (angle < -glm::half_pi<float>())
    		angle += 2 * glm::pi<float>();
    	LOGW_EVERY_N(101, angle)
    	angle = glm::clamp(angle, swc->ellipseAngleRange.x, swc->ellipseAngleRange.y);
    	// angle = tc->rotation - glm::half_pi<float>() - angle;

    	tc->rotation = angle;
    	tc->position.x = swc->ellipseParam.x * glm::cos(angle);
    	tc->position.y = swc->ellipseParam.y * glm::sin(angle);
    }
}

#if SAC_INGAME_EDITORS
void SwordSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    SwordComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "ellipse a", TW_TYPE_FLOAT, &tc->ellipseParam.x, "group=Sword");
    TwAddVarRW(bar, "ellipse b", TW_TYPE_FLOAT, &tc->ellipseParam.y, "group=Sword");
    TwAddVarRW(bar, "ellipse min angle", TW_TYPE_FLOAT, &tc->ellipseAngleRange.x, "group=Sword");
    TwAddVarRW(bar, "ellipse max angle", TW_TYPE_FLOAT, &tc->ellipseAngleRange.y, "group=Sword");
    TwAddVarRW(bar, "max angular speed", TW_TYPE_FLOAT, &tc->maxAngularSpeed, "group=Sword");
    TwAddVarRW(bar, "sword target_x", TW_TYPE_FLOAT, &tc->target.x, "group=Sword");
    TwAddVarRW(bar, "sword target_y", TW_TYPE_FLOAT, &tc->target.y, "group=Sword");
}
#endif
