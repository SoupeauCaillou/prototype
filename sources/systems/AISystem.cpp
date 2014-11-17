#include "AISystem.h"

#include "systems/TransformationSystem.h"
#include "util/Random.h"

#include "VisibilitySystem.h"
#include "UnitSystem.h"
#include "WeaponSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/ZSQDSystem.h"

INSTANCE_IMPL(AISystem);

AISystem::AISystem() : ComponentSystemImpl<AIComponent>(HASH("AI", 0x984cbb03)) {
    AIComponent tc;
    componentSerializer.add(new Property<float>(HASH("min_angle", 0xadbe6e80), OFFSET(minAngle, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("max_angle", 0xcacfe1ee), OFFSET(maxAngle, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("rotation_speed", 0x201b183c), OFFSET(rotationSpeed, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("min_pause", 0x6f40a029), OFFSET(pauses.t1, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("max_pause", 0x8b6cb1e9), OFFSET(pauses.t2, tc), 0.001f));
}

void AISystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(AI, e, uc)
        if (!UNIT(e)->alive) continue;

        Entity* weapons = UNIT(e)->weapon;

        uc->_lastSeenAccum += dt;

        if (uc->state == State::Idle && uc->_pauseAccum > 0) {
            uc->_pauseAccum -= dt;
        } else {
            float d = (uc->_targetAngle - TRANSFORM(e)->rotation);

            if (glm::abs(d) > 0.01) {
                TRANSFORM(e)->rotation += glm::sign(d) * glm::min(glm::abs(d), uc->rotationSpeed * dt);
            } else if (uc->state == State::Idle) {
                uc->_pauseAccum = uc->pauses.random();
                uc->_targetAngle = Random::Float(uc->minAngle, uc->maxAngle);
            }
        }

        if (uc->state == State::Firing && (uc->_lastSeenAccum > 2)) {
            for (int i=0; i<2; i++) WEAPON(weapons[i])->fire = false;
                uc->state = State::Idle;
                VISIBILITY(e)->raysPerFrame /= 2;
        }

        // browse visibility results
        const auto* vc = VISIBILITY(e);
        for (int i=0; i<vc->visible.count; i++) {
            Entity visible = vc->visible.entities[i];
            if (theZSQDSystem.Get(visible, false)) {
                glm::vec2 diff = TRANSFORM(visible)->position - TRANSFORM(e)->position;
                uc->_targetAngle = glm::atan(diff.y, diff.x);
                uc->_lastSeenAccum = 0;
                if (uc->state == State::Idle) {
                    uc->state = State::Firing;
                    VISIBILITY(e)->raysPerFrame *= 2;
                    WEAPON(weapons[Random::Int(0, 1)])->fire = true;
                }

                for (int i=0; i<2; i++) {
                    glm::vec2 diff2 = TRANSFORM(visible)->position - TRANSFORM(weapons[i])->position;
                    ANCHOR(weapons[i])->rotation = glm::atan(diff2.y, diff2.x) - TRANSFORM(UNIT(e)->body)->rotation;
                }
            }
        }
    END_FOR_EACH()
}

