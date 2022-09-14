
#pragma once

#include <base/EntityManager.h>
#include <glm/glm.hpp>
#include <systems/System.h>
#include <vector>

struct CornComponent {
    CornComponent() : left(1.0) {}

    float left;
};

#define theCornSystem CornSystem::GetInstance()
#define CORN(e) theCornSystem.Get(e)

UPDATABLE_SYSTEM(Corn)

public:
}
;
