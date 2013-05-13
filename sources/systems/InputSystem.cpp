#include "InputSystem.h"

#include "systems/AutoDestroySystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ParachuteSystem.h"
#include "systems/PlaneSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/DCASystem.h"
#include "util/IntersectionUtil.h"

#include <glm/gtx/norm.hpp>

INSTANCE_IMPL(InputSystem);

InputSystem::InputSystem() : ComponentSystemImpl <InputComponent>("Input") {
	InputComponent pc;
	
}

void InputSystem::DoUpdate(float) {
	FOR_EACH_ENTITY_COMPONENT(Input, entity, ic)
		switch (ic->action) {
			case Action::None:
				break;
			case Action::Spawn:
				PLANE(ic->SpawnParams.plane)->dropOne = true;
				break;
			case Action::OpenParachute: {
				Entity p = ic->OpenParachuteParams.paratrooper;
				const std::string name = PLAYER(entity)->id == 0 ? "parachute_g" : "parachute_b";
                //create a parachute
                Entity parachute = theEntityManager.CreateEntity(name,
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(name));
                TRANSFORM(parachute)->parent = p;
                PARATROOPER(p)->parachute = parachute;

                AUTO_DESTROY(p)->onDeletionCall = [] (Entity p) {
                    Entity parachute = PARATROOPER(p)->parachute;
                    if (parachute) {
                        //TRANSFORM(p)->parent = 0;
                        theEntityManager.DeleteEntity(PARACHUTE(parachute)->fils);
                        for(auto it: PARACHUTE(parachute)->holes) {
                            theEntityManager.DeleteEntity(it);
                        }
                        theEntityManager.DeleteEntity(parachute);
                    }
                };
                break;             
            }
            case Action::Fire: {
            	DCA(ic->FireParams.dca)->shoot = true;
            	DCA(ic->FireParams.dca)->targetPoint = ic->FireParams.aim;
            	break;
            }
        }
	}
}

#if SAC_INGAME_EDITORS
void InputSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    InputComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
