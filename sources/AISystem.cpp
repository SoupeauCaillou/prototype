#include "AISystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"
#include "BallSystem.h"
#include "FieldPlayerSystem.h"

INSTANCE_IMPL(AISystem);

AISystem::AISystem() : ComponentSystemImpl<AIComponent>("AI") {
    /* nothing saved */
}

void AISystem::DoUpdate(float dt) {
    const Entity ball = theBallSystem.RetrieveAllEntityWithComponent()[0];

    FOR_EACH_ENTITY_COMPONENT(AI, e, comp)
        switch (comp->state) {
            case AI::Idle:
                break;
            case AI::ReceiveBall: {
                // move toward ball
                // rough ahead of time estimate
                float ballSpeed = PHYSICS(ball)->linearVelocity.Length();
                Vector2 meToBall = TRANSFORM(ball)->worldPosition - TRANSFORM(e)->worldPosition;
                if (ballSpeed < 0.1) {
                    FIELD_PLAYER(e)->input.direction = Vector2::Normalize(meToBall);
                } else {
                    float aheadOfTime = meToBall.Length() / ballSpeed;
                    Vector2 roughFuturePos =  TRANSFORM(ball)->worldPosition + PHYSICS(ball)->linearVelocity * 0.9
                        * aheadOfTime;
                    FIELD_PLAYER(e)->input.direction = Vector2::Normalize(roughFuturePos - TRANSFORM(e)->worldPosition);
                }
                break;
            }
                
        }
    }
}

#ifdef INGAME_EDITORS
void AISystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    AIComponent* tc = Get(entity, false);
    if (!tc) return;

}
#endif
