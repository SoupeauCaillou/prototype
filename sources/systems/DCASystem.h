#pragma once

#include "systems/System.h"

#include "base/Frequency.h"

#include <glm/glm.hpp>

struct DCAComponent {
    DCAComponent() : direction(glm::vec2(1.f, 0.f)), fireRate(1.), dispersion(1.f), puissance(100.f) {}

    //direction, length doesn't matter (will be normalized)
	glm::vec2 direction;

    // in [0; +oo[, number of bullets per sec
	Frequency<float> fireRate;

    // in radians (R), dispersion angle from the direction axis. Bullets will be in [- angle/2 ; +angle/2]
    float dispersion;

    // in [0; +oo[, ejection speed of the bullets
    float puissance;
};

#define theDCASystem DCASystem::GetInstance()
#define DCA(e) theDCASystem.Get(e)

UPDATABLE_SYSTEM(DCA)
};
