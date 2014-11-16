#pragma once

#include "systems/System.h"

struct VisibilityComponent {
    VisibilityComponent(): fov(3), raysPerFrame(1), collideWith(0), _rayStartIndex(-1), _rayCount(0)  { visible.count = 0; }
    float fov; /* radians */
    /* float distance */
    int raysPerFrame;

    int collideWith;

    struct {
        Entity* entities;
        int count;
    } visible;

    int _rayStartIndex;
    int _rayCount;
};

#define theVisibilitySystem VisibilitySystem::GetInstance()
#if SAC_DEBUG
#define VISIBILITY(e) theVisibilitySystem.Get(e,true,__FILE__,__LINE__)
#else
#define VISIBILITY(e) theVisibilitySystem.Get(e)
#endif

UPDATABLE_SYSTEM(Visibility)

private:
    std::vector<Entity> rays;
    std::vector<Entity> visibles;
};
