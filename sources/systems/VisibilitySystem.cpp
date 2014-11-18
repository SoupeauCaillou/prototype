#include "VisibilitySystem.h"

#include "base/PlacementHelper.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"

#include "AISystem.h"
#include "UnitSystem.h"
#include "../LoopHelper.h"

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
        COLLISION(r)->collideWith = 9;
        *out++ = r;
    }
}

void VisibilitySystem::DoUpdate(float) {
    int resultIndex = 0;
    int raycount = 0;

    FOR_EACH_ENTITY_COMPONENT(Visibility, e, vc)
        raycount += vc->raysPerFrame;
    END_FOR_EACH()

    /* spawn new rays */
    if ((int)rays.size() < raycount) {
        int oldSize = rays.size();
        rays.resize(raycount);
        createRays(&rays[oldSize], raycount - oldSize);
        visibles.resize(raycount);
    }

    FOR_EACH_ENTITY_COMPONENT(Visibility, e, vc)
        /* read rays results from last frame */
        vc->visible.entities = &visibles[resultIndex];
        vc->visible.count = 0;

        int first = vc->_rayStartIndex;
        int latest = first + vc->_rayCount - 1;
        for (int i=first; i<=latest; i++) {
            const auto* cc = COLLISION(rays[i]);
            if (cc->collision.count) {
                LOGE_IF(cc->collision.with[0] == 0, "Entity 0 returned by raycasting");
                visibles[resultIndex + vc->visible.count] = cc->collision.with[0];
                vc->visible.count++;
            }
        }
        resultIndex += vc->visible.count;
    END_FOR_EACH()

    /* update rays */
    float angles[128];
    int rayIndex = 0;
    FOR_EACH_ENTITY_COMPONENT(Visibility, e, vc)
        LOGE_IF(vc->raysPerFrame >= 128, vc->raysPerFrame << " rays per frame won't hold in static array");
        float base = TRANSFORM(e)->rotation;
        auto* unit = UNIT(e);
        auto* ac = theAISystem.Get(e, false);
        if (ac) {
            Random::N_Floats(LoopHelper::aiRandomGenerator(unit->index), vc->raysPerFrame, angles, base - vc->fov * 0.5f, base + vc->fov * 0.5f);
        } else {
            Random::N_Floats(LoopHelper::playerRandomGenerator(unit->index), vc->raysPerFrame, angles, base - vc->fov * 0.5f, base + vc->fov * 0.5f);
        }

        int start = vc->_rayStartIndex = rayIndex;
        vc->_rayCount = vc->raysPerFrame;
        const auto& position = TRANSFORM(e)->position;

        for (int i=0; i<vc->raysPerFrame; i++) {
            Entity ray = rays[start + i];
            auto* tc = TRANSFORM(ray);
            tc->position = position;
            tc->rotation = angles[i];
            auto* cc = COLLISION(ray);
            cc->rayTestDone = false;
            cc->ignore = e;
        }
        rayIndex += vc->raysPerFrame;
    END_FOR_EACH()
}

