#include "ActionSystem.h"

#include "systems/TransformationSystem.h"

#include "systems/AnchorSystem.h"
#include "systems/RenderingSystem.h"
#include "util/IntersectionUtil.h"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(ActionSystem);

ActionSystem::ActionSystem() : ComponentSystemImpl <ActionComponent>("Action") {
    ActionComponent ac;
    componentSerializer.add(new Property<int>("action", OFFSET(action, ac), 0));
    componentSerializer.add(new EntityProperty("button", OFFSET(ClickButtonParams.button, ac)));
    componentSerializer.add(new Property<Color>("color", OFFSET(ClickButtonParams.color, ac)));
}

void ActionSystem::DoUpdate(float) {
    for (auto it: components) {
        const Entity e = it.first;
        auto* ac = it.second;

        switch (ac->action) {
            case EAction::None:
                break;
            case EAction::ClickButton:
                RENDERING(ac->ClickButtonParams.button)->color = ac->ClickButtonParams.color;
                break;
        }
    }
}
