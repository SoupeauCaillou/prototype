#include "FieldPlayerSystem.h"

#include "BallSystem.h"
#include "AISystem.h"

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
        Vector2 toBall = TRANSFORM(ball)->worldPosition - TRANSFORM(contact)->worldPosition;
        const float dist = toBall.Normalize();
        const bool ballContact = dist <= ((TRANSFORM(ball)->size.X + TRANSFORM(contact)->size.X) * 0.5);
        const bool ballOwner = (BALL(ball)->owner == player);
        Vector2& velocity = PHYSICS(player)->linearVelocity;

        Vector2& inputDirection = comp->input.direction;
        bool nokeyPressed = (inputDirection == Vector2::Zero);

        if (ballContact) {
            AI(player)->state = AI::Idle;
        }

        Vector2 moveTarget(Vector2::Zero);
        if (!ballOwner || ballContact) {
            moveTarget = inputDirection;
        } else if (ballOwner && !ballContact) {
            nokeyPressed = false;
            moveTarget = toBall;
        }

        if (comp->input.action & PASS) {
            // find nearest player
            Vector2 lookupDirection(inputDirection);
            if (lookupDirection == Vector2::Zero)
                lookupDirection = Vector2::Normalize(velocity);
            // pick nearest partner in this direction
            float min = 0;
            Entity passTarget = 0;
            const Vector2 perp(lookupDirection.Perp());
            FOR_EACH_ENTITY_COMPONENT(FieldPlayer, player2, comp2)
                if (player2 == player) continue;
                float dist = Vector2::Dot(TRANSFORM(player2)->worldPosition - TRANSFORM(player)->worldPosition, perp);
                if (!passTarget || (min < 0 && dist > 0) || (min * dist > 0 && MathUtil::Abs(dist) < MathUtil::Abs(min))) {
                    passTarget = player2;
                    min = dist;
                }
            }
            Vector2 d = TRANSFORM(passTarget)->worldPosition - TRANSFORM(player)->worldPosition;
            PHYSICS(ball)->linearVelocity = Vector2::Zero;
            PHYSICS(ball)->forces.push_back(std::make_pair(Force(d * 100, Vector2::Zero), 0.016));
            BALL(ball)->friction = -1;
            BALL(ball)->owner = 0;
            AI(passTarget)->state = AI::ReceiveBall;
        } else {
            if (nokeyPressed) {
                if (velocity.Length() > 0.1) {
                    velocity -= velocity * 20 * dt;
                } else {
                    size_t idx = ANIMATION(player)->name.find("run");
                    if (idx == 0) {
                        std::stringstream name;
                        std::cout << ANIMATION(player)->name;
                        name << "idle" << ANIMATION(player)->name.substr(3);
                        std::cout << " -> " << name.str() << std::endl;
                        ANIMATION(player)->name = name.str();
                    } else if (ANIMATION(player)->name.find("idle") != 0) {
                        ANIMATION(player)->name = directionToAnimName("idle", velocity);
                    }
                    velocity = Vector2::Zero;
                }
                if (ballContact)
                    PHYSICS(ball)->linearVelocity = Vector2::Zero;
            } else {
                velocity += moveTarget * (comp->accel * dt * weightDirChange);
                float length = velocity.Normalize();
                float maxSpeed = comp->speed;
                if (ballOwner) maxSpeed *= comp->ballSpeedDecrease;
                if (comp->input.action & SPRINT) maxSpeed *= comp->sprintBoost;
                if (length > comp->speed) length = comp->speed;
                velocity *= length;
                ANIMATION(player)->name = directionToAnimName("run", velocity);
            }

            // kick ball
            if (!nokeyPressed && ballContact) {
                if (Vector2::Dot(moveTarget, PHYSICS(ball)->linearVelocity) <= 5) {
                    BALL(ball)->owner = player;
                    float c = MathUtil::Max(0.5f, Vector2::Dot(Vector2::Normalize(velocity), moveTarget));
                    Vector2 force = moveTarget * c * PHYSICS(player)->linearVelocity.Length() * 80;
                    if (force.Length () > comp->maxForce) {
                        force.Normalize();
                        force *= comp->maxForce;
                    }
                    PHYSICS(ball)->forces.push_back(std::make_pair(Force(force, Vector2::Zero), 0.016));
                    BALL(ball)->friction = -3;
                    LOG(INFO) << "Kick !" << PHYSICS(player)->linearVelocity.Length() << "/" << force.Length();
                }
            }
        }
        comp->input.action = 0;
        comp->input.direction = Vector2::Zero;
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