#include "AISystem.h"

#include "systems/TransformationSystem.h"
#include "util/Random.h"

INSTANCE_IMPL(AISystem);

AISystem::AISystem() : ComponentSystemImpl<AIComponent>(HASH("AI", 0x984cbb03)) {
    AIComponent tc;
    componentSerializer.add(new Property<float>(HASH("min_angle", 0xadbe6e80), OFFSET(minAngle, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("max_angle", 0xcacfe1ee), OFFSET(maxAngle, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("rotation_speed", 0xcacfe1ee), OFFSET(rotationSpeed, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("min_pause", 0xcacfe1ee), OFFSET(pauses.t1, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("max_pause", 0xcacfe1ee), OFFSET(pauses.t2, tc), 0.001f));
}

void AISystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(AI, e, uc)
        if (uc->_pauseAccum > 0) {
            uc->_pauseAccum -= dt;
        } else {
            float d = (uc->_targetAngle - TRANSFORM(e)->rotation);

            if (glm::abs(d) > 0.01) {
                TRANSFORM(e)->rotation += glm::sign(d) * glm::min(glm::abs(d), uc->rotationSpeed * dt);
            } else {
                uc->_pauseAccum = uc->pauses.random();
                uc->_targetAngle = Random::Float(uc->minAngle, uc->maxAngle);
            }
        }
    END_FOR_EACH()
}

