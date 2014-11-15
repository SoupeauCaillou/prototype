#pragma once

#include "systems/System.h"

struct BulletComponent {

};

#define theBulletSystem BulletSystem::GetInstance()

#if SAC_DEBUG
#define BULLET(actor) theBulletSystem.Get(actor,true,__FILE__,__LINE__)
#else
#define BULLET(actor) theBulletSystem.Get(actor)
#endif

UPDATABLE_SYSTEM(Bullet)

};
