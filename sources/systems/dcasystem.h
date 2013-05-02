#pragma once

#include "systems/System.h"

#include <glm/glm.hpp>

struct DcaComponent {
	glm::vec2 direction;
	float fireRate;
};

#define theDcaSystem DcaSystem::GetInstance()
#define DCA(e) theDcaSystem.Get(e)

UPDATABLE_SYSTEM(Dca)
};
