#pragma once

#include "systems/System.h"

#include "base/Frequency.h"

#include <glm/glm.hpp>

struct BulletComponent {
    BulletComponent() {}
};

#define theBulletSystem BulletSystem::GetInstance()
#define BULLET(e) theBulletSystem.Get(e)

UPDATABLE_SYSTEM(Bullet)
};
