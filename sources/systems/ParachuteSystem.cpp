#include "ParachuteSystem.h"

#include "systems/AnchorSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/DebuggingSystem.h"
#include "systems/RenderingSystem.h"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/norm.hpp>
#include "util/drawVector.h"

INSTANCE_IMPL(ParachuteSystem);

ParachuteSystem::ParachuteSystem() : ComponentSystemImpl <ParachuteComponent>("Parachute") {
	ParachuteComponent pc;
    componentSerializer.add(new Property<float>("frottement", OFFSET(frottement, pc), 0.001));
    componentSerializer.add(new EntityProperty("fils", OFFSET(fils, pc)));
}

void ParachuteSystem::DoUpdate(float dt) {
    static std::vector<Entity> vectorList;
    std::for_each(vectorList.begin(), vectorList.end(), [](Entity e){
        theEntityManager.DeleteEntity(e);
    });
    vectorList.clear();

    FOR_EACH_ENTITY_COMPONENT(Parachute, e, pc)
        //find the paratrooper associated to the parachute
        const TransformationComponent* tc = TRANSFORM(e);
        const Entity paratrooper = ANCHOR(e)->parent;

        //the parachute hasn't any passenger? then destroy it
        if (! paratrooper) {
            theEntityManager.DeleteEntity(e);
            continue;
        }

        //has been totally damaged
        if (pc->damages.size() > 10) {
            PARATROOPER(paratrooper)->parachute = 0;
            ParachuteSystem::DeleteParachute(e);
            continue;
        }

        //calculate the X damage average position. Two force will be applied
        //one on the left, the other on the right, depending on the most damaged position
        float coeff[2] = {0.5, 0.5};

        if (pc->damages.size() > 0) {
            std::for_each(pc->damages.begin(), pc->damages.end(), [&](const glm::vec2 & n){
                if (n.x < 0.5)
                    coeff[0] -= 0.001;
                else
                    coeff[1] -= 0.001;
            });
        } else {
            // if the parachute is okay, forces are equals
        }

        //LOGI_EVERY_N(60, pc->damages.size() << ": damage x average position: " << xMaxDamaged << "|" << glm::cos(TRANSFORM(e)->rotation))
        glm::vec2 applicationPoint = glm::rotate(glm::vec2(tc->size.x / 2.f, 0.f), tc->rotation);

        glm::vec2 axe(tc->position - TRANSFORM(paratrooper)->position);
        axe = glm::normalize(axe);

        //if the paratrooper is over the parachute, don't add any force
        if (axe.y < 0) {
            continue;

        // Entity hole = theEntityManager.CreateEntity("hole");
        // ADD_COMPONENT(hole, Transformation);
        // ADD_COMPONENT(hole, Rendering);
        // ADD_COMPONENT(hole, AutoDestroy);
        // TRANSFORM(hole)->position = TRANSFORM(parent)->position - cursorPosition;
        // TRANSFORM(hole)->parent = parent;
        // TRANSFORM(hole)->z = 0.1;
        // TRANSFORM(hole)->size = glm::vec2(0.5f);
        // RENDERING(hole)->show = true;
        // RENDERING(hole)->zPrePass = true;
        // AUTO_DESTROY(hole)->type = AutoDestroyComponent::OUT_OF_AREA;
        // AUTO_DESTROY(hole)->params.area.position = glm::vec2(0.f);
        // AUTO_DESTROY(hole)->params.area.size = glm::vec2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
        }

        PhysicsComponent *phc = PHYSICS(paratrooper);

        float dot = glm::dot(phc->linearVelocity, axe);
        float amplitude = - dot * 0.5f * pc->frottement;

        glm::vec2 force( amplitude * axe);

        //LOGI_EVERY_N(60, "axe" << axe << " | dot " << dot << " amplitude " << amplitude <<
        //    " force1 " << force * (1 - xMaxDamaged) << " and force 2 " << force * xMaxDamaged);
        //add air resistance force on the right/left of the parachute(drag)
    	phc->addForce(- applicationPoint, force * coeff[0], dt);
        phc->addForce(applicationPoint, force * coeff[1], dt);
	}
}

#if SAC_INGAME_EDITORS
void ParachuteSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ParachuteComponent* pc = Get(entity, false);
    if (!pc) return;
	TwAddVarRW(bar, "frottement", TW_TYPE_FLOAT, &pc->frottement, "group=Parachute precision=2 step=0,01");
}
#endif

void ParachuteSystem::DeleteParachute(Entity parachute) {
    theEntityManager.DeleteEntity(PARACHUTE(parachute)->fils);
    for(auto it: PARACHUTE(parachute)->holes) {
        theEntityManager.DeleteEntity(it);
    }
    theEntityManager.DeleteEntity(parachute);
}
