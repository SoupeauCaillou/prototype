#pragma once

#include "systems/System.h"

struct VisionComponent {
    VisionComponent(): fov(3), collideWith(0) {
        vertices.pos = 0;
        vertices.count = 0;
        _rayStartIndex = _rayCount = 0;
    }
    float fov; /* radians */

    int collideWith;

    struct {
        glm::vec2* pos;
        int count;
    } vertices;

    int _rayStartIndex, _rayCount;
};

#define theVisionSystem VisionSystem::GetInstance()
#if SAC_DEBUG
#define VISION(e) theVisionSystem.Get(e,true,__FILE__,__LINE__)
#else
#define VISION(e) theVisionSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Vision)
public:
    struct RayDistance {
        Entity e;
        float d;
    };
private:
    std::vector<RayDistance> rays;
    std::vector<glm::vec2> vertices;
};
