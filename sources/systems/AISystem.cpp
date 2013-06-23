#include "AISystem.h"
#include "systems/InputSystem.h"
#include "systems/PlaneSystem.h"
#include "systems/DCASystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/TransformationSystem.h"

INSTANCE_IMPL(AISystem);

AISystem::AISystem() : ComponentSystemImpl <AIComponent>("AI") {
    AIComponent pc;
    componentSerializer.add(new Property<float>("decision_per_second", OFFSET(decisionPerSecond, pc)));
    componentSerializer.add(new Property<float>("aiming_noise", OFFSET(aimingNoise, pc), 0.001));

}

Entity myPlane(Entity entity) {
    // touched in DCA area -> fire
    FOR_EACH_ENTITY_COMPONENT(Plane, dca, dcaC)
        if (dcaC->owner == entity)
            return dca;
    }
    LOGF("Cannot find '" << entity << "' plane");
    return 0;
}

Entity myDCA(Entity entity) {
    // touched in DCA area -> fire
    FOR_EACH_ENTITY_COMPONENT(DCA, dca, dcaC)
        if (dcaC->owner == entity)
            return dca;
    }
    LOGF("Cannot find '" << entity << "' DCA");
    return 0;
}

Entity myLowestAltitudeNotDeadEnemy(Entity entity) {
    float min = 10000;
    Entity best = 0;
    FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, paraC)
        if (paraC->owner != entity && !paraC->dead) {
            if (TRANSFORM(para)->position.y < min) {
                min = TRANSFORM(para)->position.y;
                best = para;
            }
        }
    }
    return best;
}

Entity myLowestAltitudeNotDeadNoParachuteGuy(Entity entity) {
    float min = 10000;
    Entity best = 0;
    FOR_EACH_ENTITY_COMPONENT(Paratrooper, para, paraC)
        if (paraC->owner == entity && !paraC->dead && !paraC->parachuteOpened) {
            if (TRANSFORM(para)->position.y < min) {
                min = TRANSFORM(para)->position.y;
                best = para;
            }
        }
    }
    return best;
}

void AISystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(AI, entity, aiC)
        InputComponent* ic = INPUT(entity);
        aiC->accum += dt * aiC->decisionPerSecond;
        if (aiC->accum >= 1) {
            // Determine potential actions
            std::vector<Action::Enum> potentialActions;
            // 1. look up an enemy
            Entity enemy = myLowestAltitudeNotDeadEnemy(entity);
            if (enemy) {
                potentialActions.push_back(Action::Fire);
            }
            // 2. open parachute
            Entity niceGuy = myLowestAltitudeNotDeadNoParachuteGuy(entity);
            if (niceGuy) {
                potentialActions.push_back(Action::OpenParachute);
            }
            // 3. Always try to spawn
            potentialActions.push_back(Action::Spawn);

            // pick a decision at random
            ic->action = potentialActions[(int)glm::linearRand(0.0f, (float)potentialActions.size())];

            switch (ic->action) {
                case Action::Fire: {
                    LOGI("AI - open Fire !");
                    ic->FireParams.dca = myDCA(entity);
                    float noiseX = glm::linearRand(-aiC->aimingNoise * 0.5, aiC->aimingNoise * 0.5);
                    float noiseY = glm::linearRand(-aiC->aimingNoise * 0.5, aiC->aimingNoise * 0.5);
                    ic->FireParams.aim =
                        TRANSFORM(ic->FireParams.dca)->position +
                        glm::normalize(TRANSFORM(enemy)->position + glm::vec2(noiseX, noiseY)
                            - TRANSFORM(ic->FireParams.dca)->position)
                        * (DCA(ic->FireParams.dca)->maximalDistanceForActivation * 0.8f);
                    break;
                }
                case Action::Spawn:
                    LOGI("AI - spawn !");
                    ic->SpawnParams.plane = myPlane(entity);
                    break;
                case Action::OpenParachute:
                    LOGI("AI - open parachute !");
                    ic->OpenParachuteParams.paratrooper = niceGuy;
                    break;
                default:
                    LOGE("Errr, weird action choice: " << ic->action);
            }
            aiC->accum = 0;
        } else {
            ic->action = Action::None;
        }
    }
}
