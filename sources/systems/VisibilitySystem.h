#pragma once

#include "systems/System.h"

struct VisibilityComponent {
    VisibilityComponent(): fov(3), raysPerFrame(1), collideWith(0), _rayStartIndex(-1), _rayCount(0), resultStartIndex(-1), resultCount(-1) {}
    float fov; /* radians */
    /* float distance */
    int raysPerFrame;

    int collideWith;


    int _rayStartIndex;
    int _rayCount;

    int resultStartIndex;
    int resultCount;
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
