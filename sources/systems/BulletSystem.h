#pragma once

#include "systems/System.h"

struct BulletComponent {

};

#define theBulletSystem BulletSystem::GetInstance()
#define BULLET(actor) theBulletSystem.Get(actor)
UPDATABLE_SYSTEM(Bullet)

};
