#include "InputSystem.h"

#include "systems/AutoDestroySystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ParachuteSystem.h"
#include "systems/PlaneSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/ParatrooperSystem.h"
#include "systems/DCASystem.h"

#include "systems/AnchorSystem.h"
#include "systems/RenderingSystem.h"
#include "util/IntersectionUtil.h"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(InputSystem);

InputSystem::InputSystem() : ComponentSystemImpl <InputComponent>("Input") {
	InputComponent tc;
    componentSerializer.add(new Property<int>("action", OFFSET(action, tc), 0));
    componentSerializer.add(new EntityProperty("plane", OFFSET(SpawnParams.plane, tc)));
    componentSerializer.add(new EntityProperty("paratrooper", OFFSET(OpenParachuteParams.paratrooper, tc)));
    componentSerializer.add(new EntityProperty("dca", OFFSET(FireParams.dca, tc)));
    componentSerializer.add(new Property<glm::vec2>("aim", OFFSET(FireParams.aim, tc), glm::vec2(0.001f)));

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
                if (PARATROOPER(p)->parachuteOpened)
                    break;
                const std::string soldier = PLAYER(entity)->id == 0 ? "soldier_g" : "soldier_b";
				const std::string name = PLAYER(entity)->id == 0 ? "parachute_g" : "parachute_b";
                //create a parachute
                Entity parachute = theEntityManager.CreateEntity(name,
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(name));
                ANCHOR(parachute)->parent = p;
                PARATROOPER(p)->parachute = parachute;
                PARATROOPER(p)->parachuteOpened = true;
                RENDERING(p)->texture = theRenderingSystem.loadTextureFile(soldier);

                AUTO_DESTROY(p)->onDeletionCall = [] (Entity p) {
                    Entity parachute = PARATROOPER(p)->parachute;
                    if (parachute) {
                        ParachuteSystem::DeleteParachute(parachute);
                    }
                };
                break;
            }
            case Action::Fire: {
                Entity turret = DCA(ic->FireParams.dca)->turret;
                glm::vec2 d = glm::normalize(ic->FireParams.aim - TRANSFORM(turret)->position);
                ANCHOR(turret)->rotation = glm::atan2(d.y, d.x);
            	DCA(ic->FireParams.dca)->shoot = true;
            	DCA(ic->FireParams.dca)->targetPoint = ic->FireParams.aim;
            	break;
            }
        }
	}
}

#if SAC_INGAME_EDITORS
void InputSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    InputComponent* pc = Get(entity, false);
    if (!pc) return;
}
#endif
