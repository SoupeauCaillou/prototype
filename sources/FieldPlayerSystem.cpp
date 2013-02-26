#include "FieldPlayerSystem.h"

#include "BallSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/PhysicsSystem.h"
#include "util/IntersectionUtil.h"

static std::string directionToAnimName(const std::string& prefix, const Vector2& direction);

INSTANCE_IMPL(FieldPlayerSystem);

FieldPlayerSystem::FieldPlayerSystem() : ComponentSystemImpl<FieldPlayerComponent>("FieldPlayer") {
    /* nothing saved */
}

void FieldPlayerSystem::DoUpdate(float dt) {
    const float weightDirChange = 0.85;
    const Entity ball = theBallSystem.RetrieveAllEntityWithComponent()[0];

    FOR_EACH_ENTITY_COMPONENT(FieldPlayer, player, comp)
        Entity contact = FIELD_PLAYER(player)->ballContact;
        // const bool ballContact = Vector2::IntersectionUtil::rectangleRectangle(TRANSFORM(zone), TRANSFORM(ball));
        Vector2 toBall = TRANSFORM(ball)->worldPosition - TRANSFORM(contact)->worldPosition;
        const float dist = toBall.Normalize();
        const bool ballContact = dist <= ((TRANSFORM(ball)->size.X + TRANSFORM(contact)->size.X) * 0.5);
        Vector2& velocity = PHYSICS(player)->linearVelocity;

        Vector2 moveTarget(Vector2::Zero);
        bool nokeyPressed = true;
        if (!comp->ballOwner || ballContact) {
            // move little guy
            if (comp->keyPresses & UP) {
                moveTarget.Y = 1;
                nokeyPressed = false;
            } else if (comp->keyPresses & DOWN) {
                moveTarget.Y = -1;
                nokeyPressed = false;
            }
            if (comp->keyPresses & LEFT) {
                moveTarget.X = -1;
                nokeyPressed = false;
            } else if (comp->keyPresses & RIGHT) {
                moveTarget.X = 1;
                nokeyPressed = false;
            }
        } else if (comp->ballOwner) {
            nokeyPressed = false;
            const float epsilon = 0.5;
            if (dist > epsilon) {
                moveTarget = toBall;
            }
        }

        if (nokeyPressed) {
            velocity -= velocity * 20 * dt;
            if (velocity.Length() < 0.1) {
                ANIMATION(player)->name = directionToAnimName("idle", velocity);
            }
        } else {
            velocity += moveTarget * (comp->accel * dt * weightDirChange);
            float length = velocity.Normalize();
            if (length > comp->speed) length = comp->speed;
            velocity *= length;
            ANIMATION(player)->name = directionToAnimName("run", velocity);
        }

        // kick ball
        if (!nokeyPressed && ballContact) {
            if (true || Vector2::Dot(PHYSICS(player)->linearVelocity, PHYSICS(ball)->linearVelocity) <= 0) {
                LOG(INFO) << "Kick !";
                comp->ballOwner = true;
                Vector2 force = moveTarget * comp->maxForce;
                if (force.Length () > comp->maxForce) {
                    force.Normalize();
                    force *= comp->maxForce;
                }
                PHYSICS(ball)->forces.push_back(std::make_pair(Force(force, Vector2::Zero), 0.016));
            }
        }
    }
}

#ifdef INGAME_EDITORS
void FieldPlayerSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    FieldPlayerComponent* tc = Get(entity, false);
    if (!tc) return;

}
#endif

static std::string directionToAnimName(const std::string& prefix, const Vector2& direction) {
    const std::string directions[] = {"E", "NE", "N", "NW", "W", "SW", "S", "SE"};
    float angle = MathUtil::AngleFromVector(direction);
    while (angle < 0) angle += MathUtil::TwoPi;
    while (angle > MathUtil::TwoPi) angle -= MathUtil::TwoPi;
    LOG_IF(FATAL, angle < 0 || angle > MathUtil::TwoPi) << "Invalid angle value: " << angle << ", from direction: " << direction;
    float angle2 = (angle + MathUtil::PiOver4 * 0.5) / MathUtil::PiOver4;
    return prefix + directions[((int)angle2) % 8];
}