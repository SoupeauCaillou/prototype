#include "ParachuteSystem.h"

#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/DebuggingSystem.h"
#include "systems/RenderingSystem.h"

#include "util/drawVector.h"

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

    static std::vector<Entity> vectorList;
    std::for_each(vectorList.begin(), vectorList.end(), [](Entity e){
        theEntityManager.DeleteEntity(e);
    });
    vectorList.clear();

    FOR_EACH_ENTITY_COMPONENT(Parachute, e, pc)
        //find the paratrooper associated to the parachute
        Entity paratrooper = 0;
        FOR_EACH_ENTITY(Paratrooper, p)
            if (TRANSFORM(p)->parent == e) {
                paratrooper = p;
                break;
            }
        }
        //the parachute hasn't any passenger? then destroy it
        if (! paratrooper) {
            theEntityManager.DeleteEntity(e);
            continue;
        }

        //has been totally damaged
        if (pc->damages.size() > 10) {
            destroyParachute(e);
            continue;
        }

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
        glm::vec2 applicationPoint = glm::rotate(glm::vec2(TRANSFORM(e)->size.x / 2.f, 0.f), TRANSFORM(e)->worldRotation);

        glm::vec2 axe(TRANSFORM(e)->worldPosition-TRANSFORM(paratrooper)->worldPosition);
        axe = glm::normalize(axe);

        PhysicsComponent *phc = PHYSICS(e);

        //ading the mass of the paratrooper!
        phc->addForce(PHYSICS(paratrooper)->mass * PHYSICS(paratrooper)->gravity,
            -axe, dt);


        //if the paratrooper is upon the parachute, don't add any force
        if (axe.y < 0) {
            continue;

        // Entity hole = theEntityManager.CreateEntity("hole");
        // ADD_COMPONENT(hole, Transformation);
        // ADD_COMPONENT(hole, Rendering);
        // ADD_COMPONENT(hole, AutoDestroy);
        // TRANSFORM(hole)->position = TRANSFORM(parent)->worldPosition - cursorPosition;
        // TRANSFORM(hole)->parent = parent;
        // TRANSFORM(hole)->z = 0.1;
        // TRANSFORM(hole)->size = glm::vec2(0.5f);
        // RENDERING(hole)->show = true;
        // RENDERING(hole)->zPrePass = true;
        // AUTO_DESTROY(hole)->type = AutoDestroyComponent::OUT_OF_AREA;
        // AUTO_DESTROY(hole)->params.area.position = glm::vec2(0.f);
        // AUTO_DESTROY(hole)->params.area.size = glm::vec2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
        }

        float dot = glm::dot(phc->linearVelocity, axe);
        float amplitude = - dot * 0.5f * pc->frottement;

        glm::vec2 force( amplitude * axe);

        //LOGI_EVERY_N(60, "axe" << axe << " | dot " << dot << " amplitude " << amplitude <<
        //    " force1 " << force * (1 - xMaxDamaged) << " and force 2 " << force * xMaxDamaged);
        //add air resistance force on the right/left of the parachute(drag)
    	phc->addForce(force * (1 - xMaxDamaged), -applicationPoint, dt);
        phc->addForce(force * xMaxDamaged, applicationPoint, dt);

        float max = glm::length(PHYSICS(paratrooper)->mass * PHYSICS(paratrooper)->gravity);
        max = glm::max(max, glm::length(force));
        vectorList.push_back(drawVector(TRANSFORM(e)->worldPosition,
            PHYSICS(e)->mass / max * PHYSICS(e)->gravity));
        vectorList.push_back(drawVector(TRANSFORM(paratrooper)->worldPosition,
            PHYSICS(paratrooper)->mass / max * PHYSICS(paratrooper)->gravity));
        vectorList.push_back(drawVector(TRANSFORM(e)->worldPosition - applicationPoint,
            force * (1 - xMaxDamaged) / max));
        vectorList.push_back(drawVector(TRANSFORM(e)->worldPosition + applicationPoint,
            force * xMaxDamaged / max));
	}
}

#if SAC_INGAME_EDITORS
void ParachuteSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ParachuteComponent* pc = Get(entity, false);
    if (!pc) return;
	TwAddVarRW(bar, "frottement", TW_TYPE_FLOAT, &pc->frottement, "group=Parachute precision=2 step=0,01");
}
#endif
