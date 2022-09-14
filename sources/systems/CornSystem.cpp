
#include "CornSystem.h"

#include "systems/System.h"
#include "systems/TransformationSystem.h"

#include "util/SerializerProperty.h"

INSTANCE_IMPL(CornSystem);

CornSystem::CornSystem() : ComponentSystemImpl<CornComponent>(HASH("Corn", 0)) {
    CornComponent a;
    componentSerializer.add(new Property<int>(HASH("left", 0), OFFSET(left, a)));
}

void CornSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Corn, entity, c)
    if (c->left <= 0)
        continue;

    // c->left -= 0.1 * dt;
    // TRANSFORM(entity)->size = glm::vec2(c->left, c->left);
    TRANSFORM(entity)->rotation += dt;
}
}
