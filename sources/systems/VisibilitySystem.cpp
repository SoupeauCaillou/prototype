#include "VisibilitySystem.h"

#include "base/PlacementHelper.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"

#include "util/Random.h"

INSTANCE_IMPL(VisibilitySystem);

VisibilitySystem::VisibilitySystem() : ComponentSystemImpl<VisibilityComponent>(HASH("Visibility", 0x4a368e6b)) {
    VisibilityComponent vc;
    componentSerializer.add(new Property<float>(HASH("fov", 0xf7dddc94), OFFSET(fov, vc), 0.001f));
    componentSerializer.add(new Property<int>(HASH("rays_per_frame", 0xd0d2b62), OFFSET(raysPerFrame, vc), 0.001f));
    componentSerializer.add(new Property<int>(HASH("collide_with", 0x6b658240), OFFSET(collideWith, vc), 0));
}

void createRays(Entity* out, int count) {
    for (int i=0; i<count; i++) {
        Entity r = theEntityManager.CreateEntity(HASH("__/visibility_ray", 0x31f0a0dd));
        ADD_COMPONENT(r, Transformation);
        ADD_COMPONENT(r, Collision);
        COLLISION(r)->isARay = true;
        COLLISION(r)->collideWith = 8;
        *out++ = r;
    }
}

void VisibilitySystem::DoUpdate(float) {
    int resultIndex = 0;
    int raycount = 0;

    FOR_EACH_ENTITY_COMPONENT(Visibility, e, vc)
        /* read rays results from last frame */
        vc->resultStartIndex =resultIndex;
        vc->resultCount = 0;

        int first = vc->_rayStartIndex;
        int latest = first + vc->_rayCount - 1;
        for (int i=first; i<=latest; i++, vc->resultCount++) {
            visibles[resultIndex + vc->resultCount] = COLLISION(rays[i])->collidedWithLastFrame;
        }
        raycount += vc->raysPerFrame;
    END_FOR_EACH()


    /* spawn new rays */
    if ((int)rays.size() < raycount) {
        int oldSize = rays.size();
        rays.resize(raycount);
        createRays(&rays[oldSize], raycount - oldSize);
        visibles.resize(raycount);
    }

    /* update rays */
    float angles[128];
    int rayIndex = 0;
    FOR_EACH_ENTITY_COMPONENT(Visibility, e, vc)
        LOGE_IF(vc->raysPerFrame >= 128, vc->raysPerFrame << " rays per frame won't hold in static array");
        float base = TRANSFORM(e)->rotation;
        Random::N_Floats(vc->raysPerFrame, angles, base - vc->fov * 0.5f, base + vc->fov * 0.5f);

        int start = vc->_rayStartIndex = rayIndex;
        vc->_rayCount = vc->raysPerFrame;

        for (int i=0; i<vc->raysPerFrame; i++) {
            TRANSFORM(rays[start + i])->rotation = angles[i];
        }
        rayIndex += vc->raysPerFrame;
    END_FOR_EACH()

    /* reset raycast */
    for (int i=0; i<raycount; i++) {
        COLLISION(rays[i])->rayTestDone = false;
    }
}

