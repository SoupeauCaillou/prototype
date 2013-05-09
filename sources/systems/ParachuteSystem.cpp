#include "ParachuteSystem.h"

#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/RenderingSystem.h"

INSTANCE_IMPL(ParachuteSystem);

ParachuteSystem::ParachuteSystem() : ComponentSystemImpl <ParachuteComponent>("Parachute") {
	ParachuteComponent pc;
    componentSerializer.add(new Property<float>("frottement", OFFSET(frottement, pc), 0.001));
}

void ParachuteSystem::destroyParachute(Entity parachute) {
    //find the paratrooper associated to the parachute
    Entity paratrooper = 0;
    FOR_EACH_ENTITY(Paratrooper, p)
        if (TRANSFORM(p)->parent == parachute) {
            paratrooper = p;
            break;
        }
    }

    if (! paratrooper) {
        LOGE("No paratrooper associated to parachute " << parachute)
        return;
    }

    TRANSFORM(paratrooper)->position = TRANSFORM(paratrooper)->worldPosition;
    TRANSFORM(paratrooper)->z = TRANSFORM(paratrooper)->worldZ;
    TRANSFORM(paratrooper)->parent = 0;

    //should be better done than that..
    PHYSICS(paratrooper)->linearVelocity = PHYSICS(parachute)->linearVelocity;

    theEntityManager.DeleteEntity(parachute);
}

void ParachuteSystem::DoUpdate(float dt) {
	FOR_EACH_ENTITY_COMPONENT(Parachute, e, pc)



		PhysicsComponent *phc = PHYSICS(e);

        //has been totally damaged
        if (pc->damages.size() > 10) {
            destroyParachute(e);
        } else {
            //calculate the X damage average position. Two force will be applied
            //one on the left, the other on the right, depending on the most damaged position
            // xMaxDamaged = middle -> 50% left, 50% right
            // xMaXDamaged = left-middle -> 25% left, 75% right
            float xMaxDamaged = 0.f;

            if (pc->damages.size() > 0) {
                std::for_each(pc->damages.begin(), pc->damages.end(), [&](const glm::vec2 & n){
                    xMaxDamaged += n.x;
                });

                xMaxDamaged /= pc->damages.size();

                //set in [0; 1] scale
                xMaxDamaged /= TRANSFORM(e)->size.x;
            } else {
                // if the parachute is okay, forces are equals
                xMaxDamaged = 0.5f;
            }


            //LOGI_EVERY_N(60, pc->damages.size() << ": damage x average position: " << xMaxDamaged << "|" << glm::cos(TRANSFORM(e)->worldRotation))
            glm::vec2 applicationPoint = glm::vec2(TRANSFORM(e)->size.x / 2.f, 0.f);

            //find the paratrooper associated to the parachute
            Entity paratrooper = 0;
            FOR_EACH_ENTITY(Paratrooper, p)
                if (TRANSFORM(p)->parent == e) {
                    paratrooper = p;
                    break;
                }
            }
            if (! paratrooper) theEntityManager.DeleteEntity(e);
            glm::vec2 direction(TRANSFORM(e)->worldPosition-TRANSFORM(paratrooper)->worldPosition);

            //LOGI("adding force " << PHYSICS(paratrooper)->mass * PHYSICS(paratrooper)->gravity << " to" << -direction);
            phc->addForce(PHYSICS(paratrooper)->mass * PHYSICS(paratrooper)->gravity,
                -direction, dt);

            direction = glm::normalize(direction);

            float dot = glm::dot(phc->linearVelocity, direction);

            if (direction.y < 0) continue;

            float amplitude = - dot * 0.5f * pc->frottement;

            glm::vec2 force( amplitude * direction);

            //LOGI_EVERY_N(60, "direction" << direction << " | dot " << dot << " amplitude " << amplitude << " force " << force);
            //add air resistance force on the right/left of the parachute(drag)
        	phc->addForce(force * (1 - xMaxDamaged), -applicationPoint, dt);
            phc->addForce(force * xMaxDamaged, applicationPoint, dt);
        }
	}
}

#if SAC_INGAME_EDITORS
void ParachuteSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ParachuteComponent* pc = Get(entity, false);
    if (!pc) return;
	TwAddVarRW(bar, "frottement", TW_TYPE_FLOAT, &pc->frottement, "group=Parachute precision=2 step=0,01");
}
#endif
